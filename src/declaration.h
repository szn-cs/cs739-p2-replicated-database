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
#include <boost/foreach.hpp>
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

#include "consensus_interface.grpc.pb.h"
#include "database_interface.grpc.pb.h"

using namespace std;
using namespace consensus_interface;
using namespace database_interface;
using namespace grpc;
using grpc::Server, grpc::ServerBuilder, grpc::ServerContext, grpc::ServerReader, grpc::ServerWriter, grpc::Status;  // https://grpc.github.io/grpc/core/md_doc_statuscodes.html
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan, termcolor::grey;
namespace fs = std::filesystem;

namespace utility {
  // construct a relative path
  std::string constructRelativePath(std::string path, std::string rootPath);
  std::string concatenatePath(std::string base, std::string path);

  /** 
   * retrive machine's physical time using different units
   * https://stackoverflow.com/questions/21856025/getting-an-accurate-execution-time-in-c-micro-seconds
   * https://stackoverflow.com/questions/6734375/get-current-time-in-milliseconds-using-c-and-boost
  */
  std::string getClockTime();
}  // namespace utility

namespace rpc {
  /**
   * Database RPC endpoint (which the server exports on a particular port)
  */
  class DatabaseRPC : public database_interface::DatabaseService::Service {
   public:
    //  explicit DatabaseRPC() { pthread_mutex_init(&lock, NULL); }

    grpc::Status get(grpc::ServerContext*, const database_interface::GetRequest*, database_interface::GetResponse*) override;
    grpc::Status set(grpc::ServerContext*, const database_interface::SetRequest*, database_interface::Empty*) override;
    // This just returns the current log and db snapshot to the Consensus thread, to be forwarded to a recovering replica; This would only come from other servers
    // grpc::Status recovery(grpc::ServerContext*, const database_interface::RecoveryRequest*, database_interface::Recoveryresponse*) override;
  };

  /**
   * Consensus RPC endpoint (which the server exposes through a specific port)
  */
  class ConsensusRPC : public consensus_interface::ConsensusService::Service {
   public:
    grpc::Status propose(ServerContext*, const consensus_interface::Request*, consensus_interface::Response*) override;
    grpc::Status accept(ServerContext*, const consensus_interface::Request*, consensus_interface::Response*) override;
    grpc::Status success(ServerContext*, const consensus_interface::Request*, consensus_interface::Response*) override;
    grpc::Status ping(ServerContext*, const consensus_interface::Request*, consensus_interface::Response*) override;
    grpc::Status get_leader(ServerContext*, const consensus_interface::Empty*, consensus_interface::GetLeaderResponse*) override;
  };

  namespace call {
    /**
     * Database
     * Wraps RPC calls (acts as client) to the RPC endpoint (server)
    */
    class DatabaseRPCWrapperCall {
     public:
      DatabaseRPCWrapperCall(std::shared_ptr<grpc::Channel> channel)
          : stub(database_interface::DatabaseService::NewStub(channel)) {}

      /** database calls*/
      std::string get(const std::string&);
      std::string set(const std::string&, const std::string&);

      std::unique_ptr<database_interface::DatabaseService::Stub> stub;
    };

    /**
     * Consensus
     * Wraps RPC calls (client) to the RPC endpoint (server)
    */
    class ConsensusRPCWrapperCall {
     public:
      ConsensusRPCWrapperCall(std::shared_ptr<grpc::Channel> channel)
          : stub(consensus_interface::ConsensusService::NewStub(channel)){};

      /** consensus calls */
      std::string propose(const std::string&);
      std::string accept(const std::string&);
      std::string success(const std::string&);
      std::string ping(const std::string&);
      std::string get_leader();

      std::unique_ptr<consensus_interface::ConsensusService::Stub> stub;
    };

  }  // namespace call

}  // namespace rpc

namespace app {

  enum class Service {
    Database,
    Consensus
  };

}  // namespace app

namespace utility::parse {

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

    /// @brief Stream insertion operator for address.
    ///
    /// @param stream The stream into which address is being inserted.
    /// @param s The address object.
    ///
    /// @return Reference to the ostream.
    friend std::ostream& operator<<(std::ostream& stream, const Address& n) {
      return stream << "address: " << n.address
                    << ", port: " << n.port;
    }
  };

  enum Mode {
    NODE = 0,
    USER
  };

  // support for enum with boost::program_options package:
  // This class contains the value of the parsed option
  // It also contains the map from string to enum used to know wich is wich
  template <typename E>
  struct EnumOption {
    EnumOption() : value(static_cast<E>(0)) {}
    EnumOption(const E& newValue) : value(newValue) {}

    E value;
    static std::map<std::string, E> param_map;
  };

  template <typename E>
  std::map<std::string, E> EnumOption<E>::param_map;

  template <typename E>
  void validate(boost::any& value, const std::vector<std::string>& values, EnumOption<E>*, int);
  std::istream& operator>>(std::istream& in, Mode& m);  // user to parse Mode options: convert Mode to appropriate values

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
    std::vector<std::string> cluster;              // addresses of nodes in cluster
    struct flag {
      bool debug;  // debug flag
      bool leader;
    } flag;
    int timeout;
    enum Mode mode;  // can be either "user" or "node"; not sure how to use enums with the library instead.

    // TODO: fail rate, and testing configs
    // TODO: if needed
    // set acceptor list -> vector<pair<int, std::string>>; pairs of server IDs with their addresses
    // set leader -> pair<int, string>

    /** return local address with port of service (database or consensus) */
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

    /** get machines ip address (assumes a single local ip / network card) */
    static std::string getLocalIP() {
      namespace ip = boost::asio::ip;

      boost::asio::io_service ioService;
      ip::tcp::resolver resolver(ioService);

      return resolver.resolve(ip::host_name(), "")->endpoint().address().to_string();
    }

    Config() : ip(Config::getLocalIP()){};
  };

  /// @brief Makes a address given an address and port.
  Address make_address(const std::string& address_and_port);

  /// @brief Convenience function for when a 'store_to' value is being provided to typed_value.
  ///
  /// @param store_to The variable that will hold the parsed value upon notify.
  ///
  /// @return Pointer to a type_value.
  template <typename T>
  boost::program_options::typed_value<T>* make_value(T* store_to);

  /**
     * https://download.cosine.nl/gvacanti/parsing_configuration_files_c++_CVu265.pdf
     *
    */
  void parse_options(int argc, char** argv, const std::shared_ptr<Config>& config);

}  // namespace utility::parse

namespace utility::server {
  /**
     * start server for a specific gRPC service implementation
     *
     * @address: socket address structure with ip address & port components
    */
  template <class S>
  void run_gRPC_server(utility::parse::Address address);

};  // namespace utility::server

namespace app {

  template <typename C>
  struct Endpoint {
    Endpoint() = default;
    Endpoint(std::string a) : address(a), stub(std::make_unique<C>(grpc::CreateChannel(a, grpc::InsecureChannelCredentials()))) {}

    std::string address;
    std::unique_ptr<C> stub;
  };

  struct Node {
    Node() = default;  // no default constructor
    Node(std::string consensusAddress) : consensusEndpoint(Endpoint<rpc::call::ConsensusRPCWrapperCall>(consensusAddress)){};
    Node(std::string consensusAddress, std::string databaseAddress) : consensusEndpoint(Endpoint<rpc::call::ConsensusRPCWrapperCall>(consensusAddress)),
                                                                      databaseEndpoint(Endpoint<rpc::call::DatabaseRPCWrapperCall>(databaseAddress)){};
    Node(utility::parse::Address consensus, utility::parse::Address database) : Node(consensus.getAddress(), database.getAddress()){};

    Endpoint<rpc::call::ConsensusRPCWrapperCall> consensusEndpoint;
    Endpoint<rpc::call::DatabaseRPCWrapperCall> databaseEndpoint;
  };

  struct Cluster {
    static std::shared_ptr<std::map<std::string, std::unique_ptr<Node>>> memberList;  // addresses of nodes in cluster
    static std::shared_ptr<utility::parse::Config> config;

    // static void getLeader();  // return tuple(bool, Node)

    // TODO: Don't hardcode leader, this is temporary for testing the flow of a get request coming
    // from db thread. This should be found when protocol starts or whatever
    static std::string leader_;
    static pthread_mutex_t leader_mutex;
  };

  void initializeStaticInstance(std::vector<std::string> addressList, std::shared_ptr<utility::parse::Config> config);

}  // namespace app

namespace app {
  /**
   * Represents the Log datastructure for each of the nodes.
  */
  class Consensus {
   public:
    Consensus() = default;

    /** send RPC pings to all cluster nodes */
    static void initializeProtocol();
    /** create stub instances for each of the cluster nodes. */
    static void consensus_stub_rpc_setup();

    map<string, map<int, database_interface::LogEntry>> Get_Log();  // Returns current log and db snapshots
    // Methods for adding to log at different points during paxos algorithm
    void Set_Log(const string& key, int round);                                                                // Acceptor receives proposal
    void Set_Log(const string& key, int round, int p_server);                                                  // Acceptor promises proposal
    void Set_Log(const string& key, int round, int a_server, database_interface::Operation op, string value);  // Acceptor accepts proposal
    database_interface::LogEntry new_log();                                                                    // Constructs an empty log entry
    pair<string, int> Find_Max_Proposal(const string& key, int round);

    string readFromDisk(string path);
    void writeToDisk(string path, string value);

    std::string GetLeader();

    // Store log as a map of keys, in which each round number is mapped to a log entry; Once quorum is achieved, we can delete the log entry
    map<string, map<int, database_interface::LogEntry>> pax_log;
    pthread_mutex_t log_mutex;
    //bool isLeader = false;

    inline static std::shared_ptr<Consensus> instance = std::make_shared<Consensus>();  // global instance
  };

  /**
   * Represents the State Machine (where KV store that keeps the data in memory or persistent)
  */
  class Database {
   public:
    Database() = default;

    // Get, set, and delete values in the kv store
    std::pair<std::string, int> Get_KV(const string& key);
    void Set_KV(const string& key, const string& value);
    void Delete_KV(const string& key);
    map<string, string> Get_DB();

    // TODO: Lock for each entry to improve latency?
    map<string, string> kv_store;
    pthread_mutex_t data_mutex;

    inline static std::shared_ptr<Database> instance = std::make_shared<Database>();  // global instance
  };

}  // namespace app
