#ifndef MUDUO_PROTORPC2_RPCSERVER_H
#define MUDUO_PROTORPC2_RPCSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/protorpc2/RpcService.h>

namespace muduo
{
namespace net
{

class Service;

class RpcServer
{
 public:
  RpcServer(EventLoop *loop,
            InetAddress& listenAddr);

  void setThreadNum(int numThreads) 
  {
    server_.setThreadNum(numThreads);
  }

  void registerService(Service* service);
  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  TcpServer server_;
  std::map<std::string, Service*> services_;
  RpcServiceImpl metaService_;
};

} // namespace net
} // namespace muduo

#endif

