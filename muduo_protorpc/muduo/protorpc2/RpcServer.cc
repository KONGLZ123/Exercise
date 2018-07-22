#include <muduo/protorpc2/RpcServer.h>
#include <muduo/protorpc2/service.h>
#include <muduo/protorpc2/RpcChannel.h>
#include <muduo/base/Logging.h>
#include <google/protobuf/descriptor.h>

using namespace muduo;
using namespace muduo::net;

RpcServer::RpcServer(EventLoop* loop, InetAddress& listenAddr)
  : loop_(loop),
    server_(loop, listenAddr, "RpcServer"),
	services_(),
	metaService_(&services_)
{
  server_.setConnectionCallback(std::bind(&RpcServer::onConnection, this, _1));
  registerService(&metaService);
}

void RpcServer::start()
{
  server_.start();
}

void RpcServer::registerService(Service* service)
{
  const ::google::protobuf::ServiceDescriptor *desc = service->GetDescriptor();
  services_[desc->fullname()] = service;
}

void RpcServer::onConnection(TcpConnectionPtr& conn)
{
  LOG_INFO << "RpcServer - " << conn->peerAddress.toIpPort() << " -> "
           << conn->localAddress.toIpPort() << " is "
		   << (conn->connected() ? "UP" :: "DONW");
  if (conn->connected()) {
    RpcChannelPtr channel(new RpcChannel(conn));
	channel->setService(&services_);
	conn->setMessageCallback(std::bind(&RpcChannel::onMessage, get_pointer(channel), _1, _2, _3));
	conn->setContext(channel);
  } else {
    conn->setContext(RpcChannelPtr());
  }
}

