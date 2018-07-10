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
	client_.setConnectionCallback(std::bind(&RpcClient::onConnection, this, _1));
  	client_.setMessageCallback(std::bind(&RpcChannel::onMessage, get_pointer(channel_), _1, _2, _3));
  }	

  void connect(CountDownLatch *connectLatch) 	{
  	connectLatch_ = connectLatch;
	client_.connect();
  }

  Sorter::Stub *stub() { return &stub_; }

 private:
  void onConnection(const TcpConnectionPtr &conn) {
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
	  for (auto addr : address) {
	  	sorters_.push_back(new RpcClient(loop, addr));
	  } 
	}

	void connect() {
	  assert(!loop_->isInLoopThread());
	  CountDownLatch latch(static_cast<int>(sorters_.size()));
	  for (auto &client : sorters_) {
	  	client.connect(&latch);
	  }
	  latch.wait();
	}

	void run() {
	  QueryResponse stats;
	  stats.set_min(std::numeric_limits<int64_t>::max());
	  stats.set_max(std::numeric_limits<int64_t>::min());
	  getStats(&stats);
	  LOG_INFO << "stats:\n" << stats.DebugString();

      // 求平均数
	  const int64_t count = stats.count();
	  if (count > 0) {
	    LOG_INFO << "mean: " << static_cast<double>(stats.sum()) / static_cast<double>(stats.count()); 
	  }
   
      // 求中值
	  if (count <= 0) {
	    LOG_INFO << "***** No Median";
	  }
	  else {
	  
	  }
	}

 private:
  void getStats(QueryResponse *result) {
    assert(!loop_->isInLoopThread());
	CountDownLatch latch(static_cast<int>(sorters_.size()));
	for (RpcClient& sorter : sorters_) {
	  ::rpc2::Empty req;
	  sorter.stub()->Query(req, [this, result, &latch](const QueryResponsePtr& resp) {
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
		LOG_INFO << "Starting";
		EventLoopThread loop;
		median::Collector collector(loop.startLoop(), getAddress(argc, argv));
        collector.connect();
		collector.run();
	}
}


