message("/exercise")

cmake_minimum_required(VERSION 2.8)

project(muduo-protorpc C CXX)

# message(STATUS "CMAKE_BUILD_TYPE" ${CMAKE_BUILD_TYPE})
if (NOT BUILD_TYPE)
  set(BUILD_TYPE "release")
  message(STATUS "BUILD_TYPE " ${BUILD_TYPE})
endif()

if (NOT MUDUO_PATH)
  set(MUDUO_PATH "/home/$ENV{USER}/project/build/${BUILD_TYPE}-install-cpp11")
  message(STATUS "MUDUO_PATH " ${MUDUO_PATH})
endif()

set(CXX_FLAGS
 -g
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wno-error=shadow
 -Wwrite-strings
 -march=native
 -std=c++11
)
message(STATUS "CXX_FLAGS " ${CXX_FLAGS})

message(STATUS "CMAKE_CXX_FLAGS-old " ${CMAKE_CXX_FLAGS})
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS-new " ${CMAKE_CXX_FLAGS})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
message(STATUS ${PROJECT_BINARY_DIR})

find_path(Muduo_INCLUDE_DIR "muduo/base/Types.h" "${MUDUO_PATH}/include")
find_path(Muduo_LIBRARY_DIR libmuduo_net.a "${MUDUO_PATH}/lib")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${Muduo_LIBRARY_DIR})
message(STATUS "Muduo include " ${Muduo_INCLUDE_DIR})
message(STATUS "Muduo library " ${Muduo_LIBRARY_DIR})

include_directories(${PROJECT_SOURCE_DIR}) # 添加包含目录
include_directories(${PROJECT_BINARY_DIR})
message(STATUS "PROJECT_SOURCE_DIR " ${PROJECT_SOURCE_DIR})
message(STATUS "PROJECT_BINARY_DIR " ${PROJECT_BINARY_DIR})

include_directories(${Muduo_INCLUDE_DIR})
find_library(muduo_base muduo_base)  # 查找libmuduo_base.a的路径放到muduo_base变量中
find_library(muduo_net muduo_net)
find_library(muduo_protorpc_wire muduo_protorpc_wire)
find_library(muduo_cdns muduo_cdns)
message(STATUS ${muduo_base})  # 输出库目录
message(STATUS ${muduo_net})
message(STATUS ${muduo_protorpc_wire})
message(STATUS ${muduo_cdns})
link_directories(${Muduo_LIBRARY_DIR})

find_package(Boost REQUIRED)  # 查找Boost
# message(STATUS "Boost " ${Boost})
include_directories(${Boost_INCLUDE_DIR})
find_library(BOOSTTEST_LIBRARY NAMES boost_unit_test_framework)
message(STATUS "Boost_INCLUDE_DIR " ${Boost_INCLUDE_DIR})
message(STATUS "BOOSTTEST_LIBRARY " ${BOOSTTEST_LIBRARY})

find_package(Protobuf REQUIRED) # 查找protobuf
find_path(PROTOBUF_PLUGIN google/protobuf/compiler/cpp/cpp_generator.h)
message(STATUS "Protobuf " ${Protobuf})
message(STATUS "PROTOBUF_PLUGIN " ${PROTOBUF_PLUGIN})

find_path(TCMALLOC_INCLUDE_DIR google/heap-profiler.h)
find_library(TCMALLOC_LIBRARY NAMES tcmalloc_and_profiler)
if (TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBARY)
  message(STATUS "found tcmalloc")
else()
  message(STATUS "not found tcmalloc")
endif()

if (NOT muduo_protorpc_wire)
  message(SEND_ERROR "Cannot find libmuduo_protorpc_wire, did you install protobuf?")
endif()

add_subdirectory(muduo/protorpc2)

add_subdirectory(examples)





