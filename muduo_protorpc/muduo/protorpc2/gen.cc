#include <stdio.h>
#include <memory>
#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include <muduo/protorpc2/cpp_service.h>

namespace google 
{
namespace protobuf
{

namespace compiler
{
namespace cpp
{

// cpp_helpers.h
string ClassName(const Descriptor *descriptor, bool qualified);
string StripProto(const string& filename);
extern const char kThickSeparator[];
extern const char kThinSeparator[];

namespace muduorpc
{

class RpcGenerator : public CodeGenerator
{
 public:
  RpcGenerator() = default;

  bool Generate(const FileDescriptor* file,
  				const string& parameter,
				GeneratorContext* generator_context,
				string* error) const override
  {
    if (file->service_count() == 0)
	  return true;
	
	string basename = StripProto(file->name()) + ".pb";

	{
	  std::unique_ptr<io::ZeroCopyOutputStream> output(
	  	  generator_context->OpenForInsert(basename + ".h", "includes"));
      io::Printer printer(output.get(), '$');
	  printer.Print("#include <muduo/protorpc2/service.h>\n");
	  printer.Print("#include <memory>\n");
	}

	std::vector<std::unique_ptr<ServiceGenerator>> service_generators;

	for (int i = 0; i < file->service_count(); ++i) {
	  service_generators.emplace_back(
	  	  new ServiceGenerator(file->service(i), file->name(), i));
	}

	{
	  std::unique_ptr<io::ZeroCopyOutputStream> output(
	      generator_context->OpenForInsert(basename + ".h", "namespace_scope"));
	  io::Printer printer(output.get(), '$');

	  printer.Print("\n");
	  printer.Print("kThickSeparator");
	  printer.Print("\n");

	  for (int i = 0; i < file->message_type_count(); ++i) {
	    printer.Print("typedef ::std::shared_ptr<$classname> $classname$Ptr;\n",
					  "classname", ClassName(file->message_type(i), false));
	  }

	  for (int i = 0; i < file->service_count(); ++i) {
	    printer.Print("\n");
		printer.Print(kThinSeparator);
		printer.Print("\n");
		service_generators[i]->GenerateDeclarations(&printer);
	  }
	}

	{
	  std::unique_ptr<io::ZeroCopyOutputStream> output(
	      generator_context->OpenForInsert(basename + ".cc", "namespace_scope"));
      io::Printer printer(output.get(), '$');

      for (int i = 0; i < file->service_count(); ++i) {
	    printer.Print(kThickSeparator);
		printer.Print("\n");
		service_generators[i]->GenerateImplementation(&printer);
		printer.Print("\n");
	  }
	}

	return true;
  }

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(RpcGenerator);
};

} // namespace muduorpc
} // namespace cpp
} // namespace compiler
} // namespace protobuf
} // namespace google

namespace gpbc = google::protobuf::compiler;

int main(int argc, char *argv[])
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  
  gpbc::cpp::muduorpc::RpcGenerator generator;
  return gpbc::PluginMain(argc, argv, &generator);
}

