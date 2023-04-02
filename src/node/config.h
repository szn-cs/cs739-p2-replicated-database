/** config.h defins the configuration options exposed to the program */
#pragma once

#include <string>
#include <vector>

struct Config {                               // Declare options that will be allowed both on command line and in config file
  std::string port_database, port_consensus;  // RPC expotred ports
  std::vector<std::string> cluster;           // addresses of nodes in cluster
  std::string database_directory;             // directory of database data
  bool debug;                                 // debug flag

  // TODO: fail rate, and testing configs
};
