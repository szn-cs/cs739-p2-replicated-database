#---CLIENT------------------------------------------------
set(CLIENT_SRC 
  ${SOURCE_FOLDER}/user/entrypoint.cpp
  ${SOURCE_FOLDER}/node/RPCWrapperCall.cpp
)
set(CLIENT_BINARY_NAME 
  user
)

include(CheckFunctionExists)

add_executable(${CLIENT_BINARY_NAME} 
  ${CLIENT_SRC}
  ${hw_proto_srcs_consensus}
  ${hw_proto_srcs_database}
  ${hw_grpc_srcs_consensus}
  ${hw_grpc_srcs_database}
)
target_link_libraries(${CLIENT_BINARY_NAME}
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  stdc++fs
  
  ### Boost package
  # ${Boost_LIBRARIES}
)

# target_link_libraries(${CLIENT_BINARY_NAME} gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)

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

#--------------------------------------------------------






