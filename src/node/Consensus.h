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

extern Config config;

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
  /**
   * send RPC pings to all cluster nodes
  */
  static void initializeProtocol() {
    sleep(5);
    for (Node n : Consensus::cluster) {
      n.stub_consensus->ping("message");
    }
  }

  /** 
   * create stub instances for each of the cluster nodes.
  */
  static void consensus_stub_rpc_setup() {
    if (!config.flag.leader)
      return;

    for (auto it = config.cluster.begin(); it != config.cluster.end(); ++it) {
      Node n = {
          .stub_consensus = new ConsensusRPCWrapperCall(grpc::CreateChannel(*it, grpc::InsecureChannelCredentials())),
          .stub_database = new DatabaseRPCWrapperCall(grpc::CreateChannel(*it, grpc::InsecureChannelCredentials())),
          .address = *it};
      Consensus::cluster.push_back(n);
    }

    // {
    //   // Transform each config into a address via make_address, inserting each object into the vector.
    //   std::vector<Address> cluster;
    //   std::transform(config.cluster.begin(), config.cluster.end(), std::back_inserter(cluster), make_address);

    //   // Print nodes.
    //   std::copy(cluster.begin(), cluster.end(), std::ostream_iterator<Address>(std::cout, "\n"));
    // }
  }

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
