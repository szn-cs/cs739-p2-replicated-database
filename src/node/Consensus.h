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
#include "./Database.h"
#include "./RPCWrapperCall.h"
#include "./config.h"
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
 * Consensus RPC endpoint (which the server exposes through a specific port)
*/
class ConsensusRPC : public consensusInterface::ConsensusService::Service {
 public:
  grpc::Status propose(ServerContext*, const consensusInterface::Request*, consensusInterface::Response*) override;
  grpc::Status accept(ServerContext*, const consensusInterface::Request*, consensusInterface::Response*) override;
  grpc::Status success(ServerContext*, const consensusInterface::Request*, consensusInterface::Response*) override;
  grpc::Status ping(ServerContext*, const consensusInterface::Request*, consensusInterface::Response*) override;
};

struct Node {
  ConsensusRPCWrapperCall* stub_consensus;
  DatabaseRPCWrapperCall* stub_database;
  std::string address;
};

/**
 * Represents the Log datastructure for each of the nodes.
*/
class Consensus {
 public:
  static map<string, map<int, databaseInterface::LogEntry>> Get_Log();  // Returns current log and db snapshots

  // Methods for adding to log at different points during paxos algorithm
  void Set_Log(const string& key, int round);                                                               // Acceptor receives proposal
  void Set_Log(const string& key, int round, int p_server);                                                 // Acceptor promises proposal
  void Set_Log(const string& key, int round, int a_server, databaseInterface::Operation op, string value);  // Acceptor accepts proposal
  databaseInterface::LogEntry new_log();                                                                    // Constructs an empty log entry
  static pair<string, int> Find_Max_Proposal(const string& key, int round);
  string readFromDisk(string path);
  void writeToDisk(string path, string value);

  // Store log as a map of keys, in which each round number is mapped to a log entry
  // Once quorum is achieved, we can delete the log entry
  map<string, map<int, databaseInterface::LogEntry>> pax_log;
  pthread_mutex_t log_mutex;
  bool isLeader = false;
  static std::vector<Node> cluster;  // list of nodes connections
};
