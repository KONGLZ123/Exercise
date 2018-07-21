#ifndef MUDUO_PROTORPC2_RPCSERVICE_H
#define MUDUO_PROTORPC2_RPCSERVICE_H

#include <muduo/protorpc2/rpcservice.pb.h>

namespace muduo
{
namespace net
{

class RpcServiceImpl : public RpcService
{
 public:
  RpcServiceImpl(const ServiceMap* services)
    : services_(services)
  {
  }

  virtual void listRpc(const ListRpcRequest& request,
  					   const ListRpcResponse* responsePrototype,
					   const RpcDoneCallback& done);
  virtual void getService(const GetServiceRequestPtr& request,
  						  const GetServiceResponse* responsePrototype,
						  const RpcDoneCallback& done);

 private:
  const ServiceMap* services_;
};

}
}


#endif

