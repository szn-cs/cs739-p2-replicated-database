/** config.h defins the configuration options exposed to the program */
#pragma once

#include <string>
#include <vector>

struct Config {                                  // Declare options that will be allowed both on command line and in config file
  unsigned short port_database, port_consensus;  // RPC expotred ports
  std::vector<std::string> cluster;              // addresses of nodes in cluster
  std::string database_directory;                // directory of database data
  struct flag {
    bool debug;  // debug flag
    bool leader;
  } flag;
  int timeout;

  // TODO: fail rate, and testing configs
  // TODO: if needed
  // set acceptor list -> vector<pair<int, std::string>>; pairs of server IDs with their addresses
  // set leader -> pair<int, string>
};

/// @brief Address type that contains an address and port.
struct Address {
  std::string address;
  unsigned short port;

  /// @brief Constructor.
  Address(std::string address, unsigned short port)
      : address(address),
        port(port) {}
};
