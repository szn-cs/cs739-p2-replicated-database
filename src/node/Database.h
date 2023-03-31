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
#include "Consensus.h"
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

/**
 * Database RPC endpoint (which the server exports on a particular port)
*/
class DatabaseRPC : public databaseInterface::DatabaseService::Service {
 public:
  //  explicit DatabaseRPC() {
  //         pthread_mutex_init(&lock, NULL);
  //     }

  grpc::Status get(grpc::ServerContext*, const databaseInterface::Request*, databaseInterface::Response*) override;
  grpc::Status set(grpc::ServerContext*, const databaseInterface::Request*, databaseInterface::Response*) override;
  // // This just returns the current log and db snapshot to the Consensus thread, to be forwarded to a recovering replica
  // // This would only come from other servers
  // grpc::Status recovery(grpc::ServerContext*, const databaseInterface::RecoveryRequest*, databaseInterface::Recoveryresponse*) override;
};

/** 
 * Database
 * Wraps RPC calls (acts as client) to the RPC endpoint (server)
*/
class DatabaseRPCWrapperCall {
 public:
  DatabaseRPCWrapperCall(std::shared_ptr<Channel> channel);

  /** database calls*/
  std::string get(const std::string&);
  std::string set(const std::string&);

 private:
  std::unique_ptr<databaseInterface::DatabaseService::Stub> stub;
};

/**
 * Represents the State Machine (where KV store that keeps the data in memory or persistent)
*/
class Database {
 private:
  // TODO: Lock for each entry to improve latency?
  map<string, string> kv_store;
  pthread_mutex_t data_mutex;
  map<string, string> Get_DB();

 public:
  // Get, set, and delete values in the kv store
  string Get_KV(const string& key);
  void Set_KV(const string& key, string& value);
  void Delete_KV(const string& key);
};
