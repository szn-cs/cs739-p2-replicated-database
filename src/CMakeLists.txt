cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 20)

set(CLIENT_SRC 
  user/entrypoint.cpp
  node/Database.cpp
)
set(CLIENT_BINARY_NAME 
  user
)

set(SERVER_SRC 
  node/entrypoint.cpp
  node/Consensus.cpp
  node/Database.cpp
)
set(SERVER_BINARY_NAME 
  node
)


# Protobuf
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")
# Protobuf-compiler
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)

# gRPC
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto file
get_filename_component(hw_proto "./interface/consensusInterface.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)
get_filename_component(hw_proto_2 "./interface/databaseInterface.proto" ABSOLUTE)
get_filename_component(hw_proto_path_2 "${hw_proto_2}" PATH)
# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/consensusInterface.pb.cc")
set(hw_proto_srcs_2 "${CMAKE_CURRENT_BINARY_DIR}/databaseInterface.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/consensusInterface.pb.h")
set(hw_proto_hdrs_2 "${CMAKE_CURRENT_BINARY_DIR}/databaseInterface.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/consensusInterface.grpc.pb.cc")
set(hw_grpc_srcs_2 "${CMAKE_CURRENT_BINARY_DIR}/databaseInterface.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/consensusInterface.grpc.pb.h")
set(hw_grpc_hdrs_2 "${CMAKE_CURRENT_BINARY_DIR}/databaseInterface.grpc.pb.h")

add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_srcs_2}" "${hw_proto_hdrs}" "${hw_proto_hdrs_2}" "${hw_grpc_srcs}" "${hw_grpc_srcs_2}" "${hw_grpc_hdrs}" "${hw_grpc_hdrs_2}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        -I "${hw_proto_path_2}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
        "${hw_proto_2}"
      DEPENDS "${hw_proto}" "${hw_proto_2}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# custom_hw_grpc_proto
add_library(custom_hw_grpc_proto
  ${hw_grpc_srcs}
  ${hw_grpc_srcs_2}
  ${hw_grpc_hdrs}
  ${hw_grpc_hdrs_2}
  ${hw_proto_srcs}
  ${hw_proto_srcs_2}
  ${hw_proto_hdrs}
  ${hw_proto_hdrs_2})
target_link_libraries(custom_hw_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

include(CheckFunctionExists)

add_executable(${CLIENT_BINARY_NAME} 
  ${CLIENT_SRC}
  ${hw_proto_srcs}
  ${hw_proto_srcs_2}
  ${hw_grpc_srcs}
  ${hw_grpc_srcs_2}
)
target_link_libraries(${CLIENT_BINARY_NAME}
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  stdc++fs)

  # target_link_libraries(${CLIENT_BINARY_NAME} gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)

# target_link_libraries(${CLIENT_BINARY_NAME} ${Boost_LIBRARIES})

add_executable(${SERVER_BINARY_NAME} 
  ${SERVER_SRC}
  ${hw_proto_srcs} 
  ${hw_proto_srcs_2} 
  ${hw_grpc_srcs}
  ${hw_grpc_srcs_2}
)
target_link_libraries(${SERVER_BINARY_NAME} 
  ${_REFLECTION} 
  ${_GRPC_GRPCPP} 
  ${_PROTOBUF_LIBPROTOBUF} 
  stdc++fs
)




# include_directories(${BOOST_INCLUDE_DIRS})

target_include_directories(${CLIENT_BINARY_NAME} PRIVATE ${TERMCOLOR_INCLUDE_DIRS})


target_compile_options(${CLIENT_BINARY_NAME} PUBLIC -D_FILE_OFFSET_BITS=64 -Wall -Wextra -Wno-unused -Wno-unused-parameter)
target_compile_options(${CLIENT_BINARY_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++20 -D_FILE_OFFSET_BITS=64>)
# optimized: 
# target_compile_options(${CLIENT_BINARY_NAME} PUBLIC -D_FILE_OFFSET_BITS=64 -O3 -Wall -Wextra -Wno-unused -Wno-unused-parameter)
# target_compile_options(${CLIENT_BINARY_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++20 -O3 -D_FILE_OFFSET_BITS=64>)

check_function_exists(fallocate HAVE_FALLOCATE)
check_function_exists(fallocate HAVE_FLOCK)
check_function_exists(utimensat HAVE_UTIMENSAT)
check_function_exists(setxattr HAVE_XATTR)
if (${HAVE_FALLOCATE})
    target_compile_definitions(${CLIENT_BINARY_NAME} PUBLIC HAVE_FALLOCATE)
endif ()
if (${HAVE_FLOCK})
    target_compile_definitions(${CLIENT_BINARY_NAME} PUBLIC HAVE_FLOCK)
endif ()
if (${HAVE_UTIMENSAT})
    target_compile_definitions(${CLIENT_BINARY_NAME} PUBLIC HAVE_UTIMENSAT)
endif ()
if (${HAVE_XATTR})
    target_compile_definitions(${CLIENT_BINARY_NAME} PUBLIC HAVE_XATTR)
endif ()


install(TARGETS ${CLIENT_BINARY_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
