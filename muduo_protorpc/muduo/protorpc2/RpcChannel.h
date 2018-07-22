#ifndef MUDUO_PROTORPC2_RPCCHANNEL_H
#define MUDUO_PROTORPC2_RPCCHANNEL_H


typedef ::std::shared_ptr<Message> MessagePtr;

namespace muduo
{
namespace net
{

class RpcChannel : noncopyable
{
 public:
  typedef std::map<std::string, Service*> ServiceMap;
  RpcChannel();
  explicit RpcChannle(const TcpConnectionPtr& conn);
  ~RpcChannle();

  void setConnection(const TcpConnectionPtr& conn) { conn_ = conn; }

  void setServices(const ServiceMap* service) { services_ = service }
  const ServiceMap* getServices() const { return services_; }

  typedef std::bind<void (const ::google::protobuf::MessagePtr&)> ClietnDoneCallback;

  void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                  const ::google::protobuf::Message& request,
				  const ::google::protobuf::Message* response,
				  const ClientDoneCallback& done);

  void onDisconnect();
  void onMessage(const TcpConnectionPtr& conn,
                 Buffer *buf,
				 Timestamp receiveTime)			

 private:
  void onRpcMessage(const TcpConnectionPtr& conn,
					const RpcMessagePtr& messagePtr,
					Timestamp receiveTime);
  
  void callServiceMethod(const RpcMessagePtr& message);
  void doneCallback(const ::google::protobuf::Message* responsePrototype,
                    const ::google::protobuf::Message* response,
					int64_t id);

  struct OutsatndingCall
  {
    const ::google::protobuf::Message* response;
	ClientDoneCallback done;
  };
  
  RpcCodec codec_;
  TcpConnectionPtr conn_;
  AtomicInt64 id_;
  MutexLock mutex_;
  std::map<int64_t, OutstandingCall> outstandings_;
  const ServiceMap* services_;
};

} // namespace net
} // namespace muduo

#endif

