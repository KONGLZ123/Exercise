message("/median")

PROTOBUF_GENERATE_RPC(PROTO_SRCS median.proto)

add_library(median_proto ${PROTO_SRCS})
set_source_files_properties(${PROTO_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-conversion")

add_executable(median_collector collector.cc)
target_link_libraries(median_collector median_proto muduo_protorpc2)

