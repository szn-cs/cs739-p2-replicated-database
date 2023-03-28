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

#define CHUNK_SIZE 1572864

namespace fs = std::filesystem;
using namespace std;
using namespace grpc;
using namespace consensusInterface;
using namespace databaseInterface;
using grpc::Server, grpc::ServerBuilder, grpc::ServerContext, grpc::ServerReader, grpc::ServerWriter, grpc::Status;  // https://grpc.github.io/grpc/core/md_doc_statuscodes.html
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan;

class DatabaseRPC : public databaseInterface::DatabaseService::Service {
  // pthread_mutex_t lock;
 public:
  //  explicit DatabaseRPC() {
  //         pthread_mutex_init(&lock, NULL);
  //     }

  grpc::Status get(grpc::ServerContext*, const databaseInterface::Request*, databaseInterface::Response*) override;
  grpc::Status set(grpc::ServerContext*, const databaseInterface::Request*, databaseInterface::Response*) override;
};

class Database : DatabaseRPC {
  // HashMap
  //     // DB state machine

  //   set_atomically() {
  //     // Mutex locks
  // }
};

class DatabaseClientRPC {
 public:
  DatabaseClientRPC(std::shared_ptr<Channel> channel);

  /** database calls*/
  std::string get(const std::string&);
  std::string set(const std::string&);

 private:
  std::unique_ptr<databaseInterface::DatabaseService::Stub> stub;
};