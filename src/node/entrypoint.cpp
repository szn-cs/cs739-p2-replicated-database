#include "entrypoint.h"

Config config;

/// @brief Convenience function for when a 'store_to' value is being provided to typed_value.
///
/// @param store_to The variable that will hold the parsed value upon notify.
///
/// @return Pointer to a type_value.
template <typename T>
boost::program_options::typed_value<T>* make_value(T* store_to) {
  return boost::program_options::value<T>(store_to);
}

/// @brief Stream insertion operator for address.
///
/// @param stream The stream into which address is being inserted.
/// @param s The address object.
///
/// @return Reference to the ostream.
std::ostream& operator<<(std::ostream& stream, const Address& n) {
  return stream << "address: " << n.address
                << ", port: " << n.port;
}

/// @brief Makes a address given an address and port.
Address make_address(const std::string& address_and_port) {
  // Tokenize the string on the ":" delimiter.
  std::vector<std::string> tokens;
  boost::split(tokens, address_and_port, boost::is_any_of(":"));

  // If the split did not result in exactly 2 tokens, then the value
  // is formatted wrong.
  if (2 != tokens.size()) {
    using boost::program_options::validation_error;
    throw validation_error(validation_error::invalid_option_value, "cluster.address", address_and_port);
  }

  // Create a address from the token values.
  return Address(tokens[0], boost::lexical_cast<unsigned short>(tokens[1]));
}

/**
 * https://download.cosine.nl/gvacanti/parsing_configuration_files_c++_CVu265.pdf
 * 
*/
void parse_options(int argc, char** argv, Config& config) {
  namespace po = boost::program_options;  // boost https://www.boost.org/doc/libs/1_81_0/doc/html/po.html
  namespace fs = boost::filesystem;

  std::filesystem::path executablePath;
  {
    fs::path full_path(fs::initial_path<fs::path>());
    full_path = fs::system_complete(fs::path(argv[0]));
    executablePath = full_path.parent_path().string();
  }  // namespace boost::filesystem;

  po::variables_map variables;
  po::options_description cmd_options;
  po::options_description file_options;

  try {
    { /** define program options schema */
      po::options_description generic("Generic options");
      po::options_description primary("Main program options");

      generic.add_options()("help,h", "CMD options list");
      generic.add_options()("config,c", po::value<std::string>()->default_value(utility::concatenatePath(executablePath, "./node.ini")), "configuration file");
      primary.add_options()("port_database", po::value<unsigned short>(&config.port_database)->default_value(8090), "Port of database RPC service");
      primary.add_options()("port_consensus,p", po::value<unsigned short>(&config.port_consensus)->default_value(8080), "Port of consensus RPC service");
      primary.add_options()("database_directory,d", po::value<std::string>(&config.database_directory)->default_value(utility::concatenatePath(fs::current_path().generic_string(), "tmp/server")), "Directory of database data");
      primary.add_options()("cluster.address,a", make_value(&config.cluster), "Addresses (incl. ports) of consensus cluster participants <address:port>");
      primary.add_options()("flag.debug,g", po::bool_switch(&config.flag.debug)->default_value(false), "Debug flag");
      primary.add_options()("flag.leader", po::bool_switch(&config.flag.leader)->default_value(false), "testing: leader flag");
      primary.add_options()("timeout,t", po::value<int>(&config.timeout)->default_value(1), "Timeout ");

      cmd_options.add(generic).add(primary);  // set options allowed on command line
      file_options.add(primary);              // set options allowed in config file
    }

    { /** parse & set options from different sources */
      // po::store(po::parse_command_line(argc, argv, desc), vm);
      po::store(po::command_line_parser(argc, argv).options(cmd_options).run(), variables);

      // read from configuration file
      auto config_file = variables.at("config").as<std::string>();
      std::ifstream ifs(config_file.c_str());
      // if (!ifs)
      //   throw std::runtime_error("can not open configuration file: " + config_file);
      if (ifs)
        po::store(po::parse_config_file(ifs, file_options), variables);
      po::notify(variables);
      ifs.close();
    }

    if (variables.count("help")) {
      std::cout << "Distributed Replicated Database\n"
                << cmd_options << '\n'
                << endl;
      exit(0);
    }

  } catch (const po::error& ex) {
    std::cerr << red << ex.what() << reset << "\n\n";

    std::cout << "Distributed Replicated Database\n"
              << cmd_options << '\n'
              << endl;

    exit(1);
  }
}

template <typename S>
void run_gRPC_server(std::string address) {
  cout << termcolor::grey << utility::getClockTime() << termcolor::reset << endl;
  S service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

auto runDBServer = [](Address a) { run_gRPC_server<DatabaseRPC>(a.address + ":" + boost::lexical_cast<std::string>(a.port)); };
auto runConsensusServer = [](Address a) { run_gRPC_server<ConsensusRPC>(a.address + ":" + boost::lexical_cast<std::string>(a.port)); };

/**
 * initialize configurations, run RPC servers, and start consensus coordination
*/
int main(int argc, char* argv[]) {
  struct stat info;

  // parse options from different sources
  parse_options(argc, argv, config);

  // handle database directory TODO:
  fs::create_directories(fs::absolute(config.database_directory));  // create database direcotry directory if doesn't exist

  // RPC services on separate threads
  Address a1 = config.getAddress<app::Service::Consensus>(), a2 = config.getAddress<app::Service::Database>();
  std::thread t2(runConsensusServer, a1);
  std::thread t1(runDBServer, a2);
  std::cout << termcolor::blue << "⚡ Consensus service: " << a1.address + ":" + std::to_string(a1.port) << " | Database service: " << a2.address + ":" + std::to_string(a2.port) << termcolor::reset << std::endl;
  std::thread t3(Consensus::initializeProtocol);

  // start cluster coordination
  Consensus::consensus_stub_rpc_setup();

  t1.join();
  t2.join();
  t3.join();

  cout << termcolor::grey << utility::getClockTime() << termcolor::reset << endl;
  return 0;
}