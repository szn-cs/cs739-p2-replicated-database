#pragma once

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <termcolor/termcolor.hpp>
#include <vector>

#include "../Utility.cpp"
#include "consensusInterface.grpc.pb.h"
#include "databaseInterface.grpc.pb.h"

namespace fs = std::filesystem;
using namespace std;
using namespace grpc;
using namespace consensusInterface;
using namespace databaseInterface;
using grpc::Server, grpc::ServerBuilder, grpc::ServerContext, grpc::ServerReader, grpc::ServerWriter, grpc::Status;  // https://grpc.github.io/grpc/core/md_doc_statuscodes.html
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan;

/** 
 * Database
 * Wraps RPC calls (acts as client) to the RPC endpoint (server)
*/
class DatabaseRPCWrapperCall {
 public:
  DatabaseRPCWrapperCall(std::shared_ptr<grpc::Channel> channel);

  /** database calls*/
  std::string get(const std::string&);
  std::string set(const std::string&);

 private:
  std::unique_ptr<databaseInterface::DatabaseService::Stub> stub;
};

/**
 * Consensus 
 * Wraps RPC calls (client) to the RPC endpoint (server)
*/
class ConsensusRPCWrapperCall {
 public:
  ConsensusRPCWrapperCall(std::shared_ptr<grpc::Channel> channel);

  /** consensus calls */
  std::string propose(const std::string&);
  std::string accept(const std::string&);
  std::string success(const std::string&);
  std::string ping(const std::string&);

 private:
  std::unique_ptr<consensusInterface::ConsensusService::Stub> stub;
};
