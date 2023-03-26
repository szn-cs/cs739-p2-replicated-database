#pragma once

#include <errno.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <signal.h>
#include <sys/stat.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <termcolor/termcolor.hpp>

#include "grpcInterface.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientWriter;
using grpc::Status;

using namespace grpcInterface;
using grpcInterface::ReplicatedDB;

// EXAMPLE API keep it to amke sure thigns are working
using grpcInterface::Request, grpcInterface::Response;

#define TIMEOUT 60 * 1000  // this is in ms
#define CHUNK_SIZE 1572864

class GRPC_Client {
 public:
  GRPC_Client(std::shared_ptr<Channel> channel);

  std::string get(const std::string& user);

 private:
  std::unique_ptr<ReplicatedDB::Stub> stub_;
};
