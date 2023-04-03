#pragma once

#include <bits/stdc++.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <math.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <termcolor/termcolor.hpp>
#include <thread>
#include <vector>

#include "consensusInterface.grpc.pb.h"
#include "databaseInterface.grpc.pb.h"

using namespace std;
using namespace consensusInterface;
using namespace databaseInterface;
using namespace grpc;
using grpc::Server, grpc::ServerBuilder, grpc::ServerContext, grpc::ServerReader, grpc::ServerWriter, grpc::Status;  // https://grpc.github.io/grpc/core/md_doc_statuscodes.html
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan, termcolor::grey;
namespace fs = std::filesystem;

/// @brief Address type that contains an address and port.
struct Address {
  std::string address;
  unsigned short port;

  std::string getAddress() {
    return this->address + ":" + std::to_string(this->port);
  }

  /// @brief Constructor.
  Address(std::string address, unsigned short port)
      : address(address),
        port(port) {}
};

namespace parse {

}  // namespace parse

namespace app {

enum class Service {
  Database,
  Consensus
};

template <typename S>
struct Endpoint {
  Endpoint(std::string a) : address(a) {
    // this->stub = new ConsensusRPCWrapperCall(grpc::CreateChannel(a, grpc::InsecureChannelCredentials()));
  }

  std::string address;
  std::unique_ptr<S> stub;
};

class Node {
 public:
  Node(std::string consensusAddress);
  Node(std::string consensusAddress, std::string databaseAddress);
  Node(Address consensus, Address database);

  //  private:
  //   Endpoint<consensusInterface::ConsensusService::Stub> consensusEndpoint;
  //   Endpoint<databaseInterface::DatabaseService::Stub> databaseEndpoint;
};

class Cluster {
 public:
  Cluster(std::vector<std::string> addressList);
  // void getLeader();  // return tuple(bool, Node)

 public:
  std::vector<Node> memberList;  // addresses of nodes in cluster
  // Option option;
};

}  // namespace app

/**
 * Configuration info
 * 
 * options input priority/hierarchy: 
 *  1. command-line arguments
 *  2. configuration file
 *  3. default hardcoded values
*/
struct Config {  // Declare options that will be allowed both on command line and in config file
  std::string ip;
  unsigned short port_database, port_consensus;  // RPC expotred ports
  std::string database_directory;                // directory of database data

  std::vector<std::string> cluster;  // addresses of nodes in cluster

  struct flag {
    bool debug;  // debug flag
    bool leader;
  } flag;

  int timeout;

  // TODO: fail rate, and testing configs
  // TODO: if needed
  // set acceptor list -> vector<pair<int, std::string>>; pairs of server IDs with their addresses
  // set leader -> pair<int, string>

  /**
   * return local address with port of service (database or consensus) 
  */
  template <app::Service s>
  Address getAddress() {
    unsigned short p;

    switch (s) {
      case app::Service::Database:
        p = this->port_database;
        break;
      case app::Service::Consensus:
        p = this->port_consensus;
        break;
    }

    return Address{this->ip, boost::lexical_cast<unsigned short>(p)};
  }

  /**
   * get machines ip address
   * (assumes a single local ip / network card)
  */
  static std::string getLocalIP() {
    namespace ip = boost::asio::ip;

    boost::asio::io_service ioService;
    ip::tcp::resolver resolver(ioService);

    return resolver.resolve(ip::host_name(), "")->endpoint().address().to_string();
  }

  Config() : ip(Config::getLocalIP()){};
};

struct utility {
  // construct a relative path
  static std::string constructRelativePath(std::string path, std::string rootPath);
  static std::string concatenatePath(std::string base, std::string path);

  /** 
 * retrive machine's physical time using different units
 * https://stackoverflow.com/questions/21856025/getting-an-accurate-execution-time-in-c-micro-seconds
 * https://stackoverflow.com/questions/6734375/get-current-time-in-milliseconds-using-c-and-boost
*/
  static std::string getClockTime();
};

template <typename T>
boost::program_options::typed_value<T>* make_value(T* store_to);

std::ostream& operator<<(std::ostream& stream, const Address& n);

Address make_address(const std::string& address_and_port);

void parse_options(int argc, char** argv, Config& config);

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
 * Represents the State Machine (where KV store that keeps the data in memory or persistent)
*/
class Database {
 private:
  // TODO: Lock for each entry to improve latency?
  map<string, string> kv_store;
  pthread_mutex_t data_mutex;

 public:
  // Get, set, and delete values in the kv store
  string Get_KV(const string& key);
  void Set_KV(const string& key, string& value);
  void Delete_KV(const string& key);
  map<string, string> Get_DB();
};

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
  static void initializeProtocol();

  /** 
     * create stub instances for each of the cluster nodes.
    */
  static void consensus_stub_rpc_setup();

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
