/** config.h defins the configuration options exposed to the program */
#pragma once

#include <boost/asio.hpp>
#include <string>
#include <vector>

namespace app {
enum class Service {
  Database,
  Consensus
};
}

/// @brief Address type that contains an address and port.
struct Address {
  std::string address;
  unsigned short port;

  /// @brief Constructor.
  Address(std::string address, unsigned short port)
      : address(address),
        port(port) {}
};

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
