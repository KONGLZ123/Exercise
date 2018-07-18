#ifndef MUDUO_PROTORPC2_CPP_SERVICE_H
#define MUDUO_PROTORPC2_CPP_SERVICE_H


#include <map>
#include <string>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/descriptor.h>


namespace google {
namespace protobuf {
  namespace io {
    class Printer;    // printer.h
  }
}

namespace protobuf {
namespace compiler {
namespace cpp {
namespace muduorpc {

class ServiceGenerator {
 public:
  explicit ServiceGenerator(const ServiceDescriptor* descriptor,
  							const string& filename,
							int index);
  ~ServiceGenerator();

  void GenerateDeclarations(io::Printer* printer);

  void GenerateDescriptorInitializer(io::Printer* printer, int index);

  void GenerateImplementation(io::Printer* printer);

 private:
  enum RequestOrResponse { REQUEST, RESPONSE };
  enum VirtualOrNon { VIRTUAL, NON_VIRTUAL };
  enum StubOrNon { STUB, NON_STUB };

  void GenerateInterface(io::Printer* printer);

  void GenerateStubDefinition(io::Printer* printer);

  void GenerateMethodSignatures(StubOrNon stub_or_non, io::Printer* printer);

  void GenerateNotImplementedMethods(io::Printer* printer);

  void GenerateCallMethod(io::Printer* printer);

  void GenerateGetPrototype(RequestOrResponse which, io::Priner* printer);

  void GenerateStubMethods(io::Priner* printer);

  const ServiceDescriptor* descriptor_;
  map<string, string> vars_;

  GOOGLE_DISALLOW_EVIL_CONSTRUTCTORS(ServiceGenerator);
};


} // namespace muduorpc
} // namespace cpp
} // namespace compiler
} // namespace protobuf
} // namespace google

#endif
