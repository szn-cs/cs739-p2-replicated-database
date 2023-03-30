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

class Database : DatabaseRPC {
 private:
  // TODO: Lock for each entry to improve latency?
  map<string, string> kv_store;
  pthread_mutex_t data_mutex;

  // Store log as a map of keys, in which each round number is mapped to a log entry
  // Once quorum is achieved, we can delete the log entry
  map<string, map<int, databaseInterface::LogEntry>> pax_log;
  pthread_mutex_t log_mutex;

  // Constructs an empty log entry
  databaseInterface::LogEntry new_log();

 public:
  // Get, set, and delete values in the kv store
  string Get_KV(const string& key);
  void Set_KV(const string& key, string& value);
  void Delete_KV(const string& key);

  // Returns current log and db snapshots
  map<string, map<int, databaseInterface::LogEntry>> Get_Log();
  map<string, string> Get_DB();

  // Methods for adding to log at different points during paxos algorithm
  void Set_Log(const string& key, int round);                                                               // Acceptor receives proposal
  void Set_Log(const string& key, int round, int p_server);                                                 // Acceptor promises proposal
  void Set_Log(const string& key, int round, int a_server, databaseInterface::Operation op, string value);  // Acceptor accepts proposal
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
