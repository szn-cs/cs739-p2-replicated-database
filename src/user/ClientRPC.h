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

#include "consensusInterface.grpc.pb.h"
#include "databaseInterface.grpc.pb.h"

using namespace std;
using namespace consensusInterface;
using namespace databaseInterface;
using databaseInterface::Request, databaseInterface::Response;
using grpc::Channel, grpc::ClientContext, grpc::ClientReader, grpc::ClientWriter, grpc::Status;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan;

/**
 * GRPC user functionality
 * 
 * methods call generated gRPC stub to request remote gRPC functions
*/
class ClientRPC {
 public:
  ClientRPC(std::shared_ptr<Channel> channel);

  std::string get(const std::string&);
  std::string set(const std::string&);

 private:
  std::unique_ptr<databaseInterface::DatabaseService::Stub> stub;  // gRPC stub
};
