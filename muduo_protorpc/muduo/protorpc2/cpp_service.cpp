#include <muduo/protorpc2/cpp_service.h>
#include <muduo/protorpc2/io/printer.h>

namespace google {
namespace protobuf {

string simpleItoa(int i);

namespace compiler {
namespace cpp {

string ClassName(const Descriptor* descriptor, bool qualified);

namespace muduorpc {

ServiceGenerator::ServiceGenerator(const ServiceDescriptor* descriptor,
								   const string& filename,
								   int index)
  : descriptor_(descriptor) {
  vars_["classname"] = descritptor_->name();
  vars_["full_name"] = descritptor_->full_name();
  vars_["dllexport"] = "";
  vars_["filename"] = filename;
  vars_["index"] = SimpleItoa(index);
}

ServiceGenerator::~ServiceGenerator() {}

void ServiceGenerator::GeneratorDeclarations(io::Printer* printer) {
  printer->Print(vars_, 
    "class $classname$_Stub;\n"
	"\n");

  GenerateInterface(printer);
  GenerateStubDefinition(printer);
}

void ServiceGenerator::GeneratorInterface(io::Printer* printer) {
  printer->Print(vars_,
    "class $dllexport$$classname$ : public ::muduo::net::Service {\n"
	" protected:\n"
	"  inline $classname$(){};\n"
	" public:\n"
	"  virtual ~$classname$();\n");
  printer->Indent();

  printer->Print(vars_,
    "\n"
	"typedef $classname$_Stub Stub;\n"
	"\n"
	"static const ::google::protobuf::ServiceDescriptor* descritptor();\n"
	"\n");

  GenerateMethodSignatures(NON_STUB, printer);

  printer->Print(
    "\n"
	"const ::google::protobuf::ServiceDescriptor* GetDescriptor();\n"
	"void CallMethod(const ::google::protobuf::MethodDescriptor* method,\n"
	"                const ::google::protobuf::MessagePtr& request,\n"
	"                const ::google::protobuf::Message* responsePrototype,\n"
	"                const ::muduo::net::RpcDoneCallback& done);\n"
	"const ::google::protobuf::Message& GetRequestPrototype(\n"
	"    const ::google::protobuf::MethodDescriptor* method) const;\n"
	"const ::google::protobuf::Message& GetResponsePrototype(\n"
	"    const ::google::protobuf::MethodDescriptor* method) const;\n");

  printer->Outdent();
  printer->Print(vars_,
    "\n"
	" private:\n"
	"  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS($classname$);\n"
	"};\n"
	"\n");
}

void ServiceGenerator::GenerateStubDefinition(io::Printer* printer)
{
  
}

} // namespace muduorpc

} // namespace cpp
} // namespace compiler
} // namespace protobuf
} // namespace google



