#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Logging.h>
#include <muduo/protorpc2/RpcServer.h>
#include "median.pb.h"

using namespace muduo;
using namespace muduo::net;

bool debug = false;

namespace median
{

class SorterImpl : public Sorter
{
 public:
  SorterImpl() 
  {
    xsubi_[0] = static_cast<unsigned short>(getpid());
    xsubi_[1] = static_cast<unsigned short>(gethostid());
    xsubi_[2] = static_cast<unsigned short>(Timestamp::now().microSecondsSinceEpoch());
    doGenerate(100, 0, 30);
  }

  void Query(const ::rpc2::EmptyPtr& request,
  			 const QueryResponse* responsePrototype,
			 const RpcDoneCallback& done) override
  {
  	LOG_INFO << "--Query";
  	
	QueryResponse resp;
	resp.set_count(data_.size());
	LOG_INFO << "count: " << data_.size();
	if (data_.size() > 0) {
	  resp.set_min(data_[0]);
	  resp.set_max(data_.back());
	  resp.set_sum(std::accumulate(data_.begin(), data_.end(), 0LL));
	  
	  LOG_INFO << " count: " << resp.count()
	  		   << " min: " << resp.min() 
			   << " max: " << resp.max() 
			   << " sum: " << resp.sum();
	}
	done(&resp);
  }

  void Search(const SearchRequestPtr& request,
   			  const SearchResponse* responsePrototype,
			  const RpcDoneCallback& done) override
  {
    int64_t guess = request->guess();
	LOG_INFO << "--Search " << guess;

	SearchResponse resp;
    std::vector<int64_t>::iterator it = std::lower_bound(data_.begin(), data_.end(), guess);
	resp.set_smaller(it - data_.begin());
	resp.set_same(std::upper_bound(data_.begin(), data_.end(), guess) - it);

    LOG_INFO << "smaller: " << resp.smaller() 
	         << "same: " << resp.same();
  	done(&resp);
  }

 private:
  void doGenerate(int count, int min, int max) {
    data_.clear();
	LOG_INFO << "value: ";
    for (int i = 0; i < count; ++i) {
      int64_t range = max - min;
      int64_t value = min;
      if (range > 1)
		value += nrand48(xsubi_) % range;
      data_.push_back(value);
      //LOG_INFO << value;
      std::sort(data_.begin(), data_.end());
      if (debug) {
        std::copy(data_.begin(), data_.end(), std::ostream_iterator<int64_t>(std::cout, " "));
		std::cout << std::endl;
      }
    }
  }
  std::vector<int64_t> data_;
  unsigned short xsubi_[3]; 
};

}

int main(int argc, char *argv[])
{
  EventLoop loop;
  int port = argc > 1 ? atoi(argv[1]) : 5555;
  debug = argc > 2;
  InetAddress listenAddr(static_cast<uint16_t>(port));
  median::SorterImpl impl;
  RpcServer server(&loop, listenAddr);
  server.registerService(&impl);
  server.start();
  loop.loop();
  google::protobuf::ShutdownProtobufLibrary();
}



