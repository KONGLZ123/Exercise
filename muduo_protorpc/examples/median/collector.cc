#include <vector>
#include <string>
#include <functional>
#include <muduo/base/Logging.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <boost/core/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <examples/median/median.pb.h>
#include <examples/median/kth.h>

using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;  // for _1, _2, _3...

namespace median
{

class RpcClient : boost::noncopyable
{
 public:
  RpcClient(EventLoop *loop, InetAddress &addr)
  	: loop_(loop),
  	  connectLatch_(NULL),
	  client_(loop, addr, "RpcClient " + addr.toIpPort()),
	  channel_(new RpcChannel),
	  stub_(get_pointer(channel_))
  {
    LOG_INFO << "--RpcClient ctor";
	client_.setConnectionCallback(std::bind(&RpcClient::onConnection, this, _1));
  	client_.setMessageCallback(std::bind(&RpcChannel::onMessage, get_pointer(channel_), _1, _2, _3));
  }	

  void connect(CountDownLatch *connectLatch) 	{
    LOG_INFO << "--connect";
  	connectLatch_ = connectLatch;
	client_.connect();
  }

  Sorter::Stub *stub() { return &stub_; }

 private:
  void onConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "--onConnection";
  	if (conn->connected()) {
	  channel_->setConnection(conn);
	  if (connectLatch_) {
	  	connectLatch_->countDown();
		connectLatch_ = NULL;
	  }
	}
  }

  EventLoop *loop_;
  CountDownLatch *connectLatch_;	
  TcpClient client_;
  RpcChannelPtr channel_;
  Sorter::Stub stub_;
};

class Collector : boost::noncopyable
{
 public:
  	Collector(EventLoop* loop, const std::vector<InetAddress>& address)
		: loop_(loop)		
	{
	  LOG_INFO << "--Collector constructor";
 	  for (auto addr : address) {
	  	sorters_.push_back(new RpcClient(loop, addr));
	  } 
	}

	void connect() {
	  LOG_INFO << "--connect";
	  assert(!loop_->isInLoopThread());
	  CountDownLatch latch(static_cast<int>(sorters_.size()));
	  for (auto &client : sorters_) {
	  	client.connect(&latch);
	  }
	  latch.wait();
	}

	void run() {
	  LOG_INFO << "--run";
	  QueryResponse stats;
	  stats.set_min(std::numeric_limits<int64_t>::max());
	  stats.set_max(std::numeric_limits<int64_t>::min());
	  getStats(&stats);
	  LOG_INFO << "stats:\n" << stats.DebugString();

      // 求平均数
	  const int64_t count = stats.count();
	  LOG_INFO << "count: " << count;
	  if (count > 0) {
	    LOG_INFO << "mean: " << static_cast<double>(stats.sum()) / static_cast<double>(stats.count()); 
	  }
   
      // 求中值
	  if (count <= 0) {
	    LOG_INFO << "***** No Median";
	  }
	  else {
        const int64_t k = (count + 1) / 2;
		std::pair<int64_t, bool> median = getKth(std::bind(&Collector::search, this, _1, _2, _3), k, count, stats.min(), stats.max());
		if (median.second) {
		  LOG_INFO << "********** Median is " << median.first;
		}
		else {
		  LOG_ERROR << "********************** Median not found";
		}
	  }
	}

 private:
  void getStats(QueryResponse *result) {
    LOG_INFO << "--getStats";
    assert(!loop_->isInLoopThread());
	CountDownLatch latch(static_cast<int>(sorters_.size()));
	LOG_INFO << "sorters_.size(): " << sorters_.size();
	for (RpcClient& sorter : sorters_) {
	  ::rpc2::Empty req;
	  LOG_INFO << "for RpcClient";
	  sorter.stub()->Query(req, [this, result, &latch](const QueryResponsePtr& resp) {
	    LOG_INFO << "call Query";
		assert(loop_->isInLoopThread());
		result->set_count(result->count() + resp->count());
		result->set_sum(result->sum() + resp->sum());
        if (resp->count() > 0) {
		  if (resp->min() < result->min()) 
		    result->set_min(resp->min());
		  if (resp->max() > result->max())
		    result->set_max(resp->max());
		}
		latch.countDown();
	  });
    }
	latch.wait();
  }

  void search(int64_t guess, int64_t* smaller, int64_t* same) {
    LOG_INFO << "--search";
    assert(!loop_->isInLoopThread());
	*smaller = 0;
	*same = 0;
	CountDownLatch latch(static_cast<int>(sorters_.size()));
	for (RpcClient& sorter : sorters_) {
      SearchRequest req;
	  req.set_guess(guess);
	  sorter.stub()->Search(req, [this, smaller, same, &latch](const SearchResponsePtr& resp) { 
          LOG_INFO << "call Search";
		  assert(loop_->isInLoopThread());
		  *smaller += resp->smaller();
		  *same += resp->same();
		  latch.countDown();
		});
	}
	latch.wait();
  }

  EventLoop *loop_; 
  boost::ptr_vector<RpcClient> sorters_;
};

}

const std::vector<InetAddress> getAddress(int argc, char *argv[])
{
	std::string addr;
	std::vector<InetAddress> result;
	for (int i = 1; i < argc; ++i) {
		addr = argv[i];
		size_t pos = addr.find(":");
		if (pos != std::string::npos) {	
			std::string ip(addr.substr(0, pos));
			uint16_t port = static_cast<uint16_t>(atoi(addr.substr(pos + 1, addr.length()).c_str()));
			result.push_back(InetAddress(ip, port));
			LOG_INFO << "ip:" << ip << " port:" << port;
		}
		else {
			LOG_ERROR << "Invalid address:" << addr;
		}
	}
	return result;
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		LOG_INFO << "--Collector Starting";
		EventLoopThread loop;
		median::Collector collector(loop.startLoop(), getAddress(argc, argv));
        collector.connect();
	    LOG_INFO << "--All connected";
	    collector.run();
	}
	else {
	  printf("Usage: %s sorter_addresses\n", argv[0]);
	}
}


