#include "entrypoint.h"

Config config;

static std::unordered_map<std::string, int> serverclock;
static std::string serverDirectory;

template <typename S>
void run_gRPC_server(std::string address) {
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
  std::cout << termcolor::blue << "⚡ Server listening on " << address << termcolor::reset << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

auto runConsensusServer = [](std::string address) { run_gRPC_server<DatabaseRPC>(address); };
auto runDBServer = [](std::string address) { run_gRPC_server<ConsensusRPC>(address); };

void parse_options(int argc, char** argv) {
  namespace po = boost::program_options;  // boost https://www.boost.org/doc/libs/1_81_0/doc/html/po.html
  namespace fs = boost::filesystem;

  std::filesystem::path executablePath;
  {
    fs::path full_path(fs::initial_path<fs::path>());
    full_path = fs::system_complete(fs::path(argv[0]));
    executablePath = full_path.parent_path().string();
  }  // namespace boost::filesystem;

  try {
    po::options_description cmd_options;
    po::options_description file_options;

    { /** define program options schema */
      po::options_description generic("Generic options");
      po::options_description primary("Main program options");

      generic.add_options()("help,h", "CMD options list");
      generic.add_options()("config,c", po::value<std::string>()->default_value(Utility::concatenatePath(executablePath, "./node.config")), "configuration file");
      primary.add_options()("port_database,p", po::value<std::string>(&config.port_database)->default_value("8080"), "Port of database RPC service");
      primary.add_options()("port_consensus", po::value<std::string>(&config.port_consensus)->default_value("8081"), "Port of consensus RPC service");
      primary.add_options()("address,a", po::value<std::vector<std::string>>(&config.cluster)->multitoken(), "Addresses (incl. ports) of consensus cluster participants");
      primary.add_options()("database_directory,d", po::value<std::string>(&config.database_directory)->default_value(Utility::concatenatePath(fs::current_path().generic_string(), "tmp/server")), "Directory of database data");
      primary.add_options()("debug,g", po::bool_switch(&config.debug)->default_value(false), "Debug flag");

      cmd_options.add(generic).add(primary);  // set options allowed on command line
      file_options.add(primary);              // set options allowed in config file
    }

    po::variables_map variables;
    { /** parse & set options from different sources */
      // po::store(po::parse_command_line(argc, argv, desc), vm);
      po::store(po::command_line_parser(argc, argv).options(cmd_options).run(), variables);

      // read from configuration file
      auto config_file = variables.at("config").as<std::string>();
      std::ifstream ifs(config_file.c_str());
      // if (!ifs)
      //   throw std::runtime_error("can not open configuration file: " + config_file);
      if (ifs)
        po::store(po::parse_config_file(ifs, config_file), variables);
      po::notify(variables);
    }

    if (variables.count("help")) {
      std::cout << "Distributed Replicated Database\n"
                << cmd_options << '\n'
                << endl;
      exit(0);
    }

  } catch (const po::error& ex) {
    std::cerr << ex.what() << '\n';
    exit(1);
  }
}

int main(int argc, char* argv[]) {
  struct stat info;

  parse_options(argc, argv);

  {
    // TODO: handle database directory
    // create database direcotry directory if doesn't exist
    fs::create_directories(fs::absolute(config.database_directory));
  }

  std::thread consensus(runDBServer, "0.0.0.0:" + config.port_database);
  std::thread db(runConsensusServer, "0.0.0.0:" + config.port_consensus);

  db.join();
  consensus.join();

  return 0;
}