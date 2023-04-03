#pragma once

#include "../include.h"
#include "../utility.h"
#include "./Database.h"
#include "./RPCWrapperCall.h"
#include "./config.h"

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
  grpc::Status get_leader(ServerContext*, const consensusInterface::Empty*, consensusInterface::GetLeaderResponse*) override;
};

struct Node {
  ConsensusRPCWrapperCall* stub_consensus;
  DatabaseRPCWrapperCall* stub_database;
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
    for (auto& n : cluster) {
      n.second.stub_consensus->ping("message");
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
          .stub_database = new DatabaseRPCWrapperCall(grpc::CreateChannel(*it, grpc::InsecureChannelCredentials()))};

      cluster[*it] = n;
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

  // Store log as a map of keys, in which each round number is mapped to a log entry
  // Once quorum is achieved, we can delete the log entry
  map<string, map<int, databaseInterface::LogEntry>> pax_log;
  pthread_mutex_t log_mutex;
  //bool isLeader = false;

  static string leader_;
  static pthread_mutex_t leader_mutex;

  static std::map<std::string, Node> cluster;  // map of addresses to stubs
  pthread_mutex_t cluster_mutex;

  string readFromDisk(string path);
  void writeToDisk(string path, string value);

  static std::string GetLeader();
};
