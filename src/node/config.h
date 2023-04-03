/** config.h defins the configuration options exposed to the program */
#pragma once

#include "../include.h"
#include "./RPCWrapperCall.h"

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