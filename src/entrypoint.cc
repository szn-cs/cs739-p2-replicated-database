#include "./declaration.h"

int user_entrypoint(std::shared_ptr<utility::parse::Config> config, boost::program_options::variables_map& variables);

/**
 * initialize configurations, run RPC servers, and start consensus coordination
 */
int main(int argc, char* argv[]) {
  cout << termcolor::grey << utility::getClockTime() << termcolor::reset << endl;

  struct stat info;
  std::shared_ptr<utility::parse::Config> config = std::make_shared<utility::parse::Config>();

  boost::program_options::variables_map variables;
  utility::parse::parse_options<utility::parse::Mode::NODE>(argc, argv, config, variables);  // parse options from different sources

  // Initialize Cluster data & Node instances
  app::initializeStaticInstance(config->cluster, config);

  /** pick Mode of opeartion: either run distributed database or run the user testing part. */
  switch (config->mode) {
    case utility::parse::Mode::USER: {
      // additional parsing
      utility::parse::parse_options<utility::parse::Mode::USER>(argc, argv, config, variables);  // parse options from different sources
      return user_entrypoint(config, variables);
    } break;
    case utility::parse::Mode::NODE:
    default:
      // continue
      break;
  }

  // handle database directory TODO:
  fs::create_directories(fs::absolute(config->database_directory));  // create database direcotry directory if doesn't exist

  // RPC services on separate threads
  utility::parse::Address a1 = config->getAddress<app::Service::Consensus>();
  std::thread t1(utility::server::run_gRPC_server<rpc::ConsensusRPC>, a1);
  utility::parse::Address a2 = config->getAddress<app::Service::Database>();
  std::thread t2(utility::server::run_gRPC_server<rpc::DatabaseRPC>, a2);
  std::cout << termcolor::blue << "⚡ Consensus service: " << a1.toString() << " | Database service: " << a2.toString() << termcolor::reset << std::endl;
  // ping broadcasting thread (every randomly picked interval)
  //std::thread t3(app::Consensus::broadcastPeriodicPing);

  // start cluster coordination
  assert(app::Consensus::coordinate().ok());

  t1.join();
  t2.join();
  //t3.join();

  cout << termcolor::grey << utility::getClockTime() << termcolor::reset << endl;
  return 0;
}

/** User execution for testing the RPC servers */
int user_entrypoint(std::shared_ptr<utility::parse::Config> config, boost::program_options::variables_map& variables) {
  // TODO: parse options for sending key value pair to address of cluster member for testing purposes through terminal.

  //   ./target/app --mode user --command set --key k1 --value v1 --target 0.0.0.0:8002
  if (variables.count("command")) {  // if command exists
    auto target = variables.at("target").as<std::string>();
    auto command = variables.at("command").as<std::string>();
    auto key = variables.at("key").as<std::string>();
    auto value = variables.at("value").as<std::string>();

    std::cout << command + " " + key + " " + value + " send to " + target << std::endl;
    // TODO: create RPC requests to modify the database.

    int r = 0;
    //string db_address = "127.0.1.1:9000";  // target address & port to send grpc requests to.

    rpc::call::DatabaseRPCWrapperCall* c = new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));

    if(command == "set"){
      c->set(key, value);
    }else if (command == "get"){
      string message = c->get(key);
      std::cout << message << std::endl;
    } // TODO: Delete? Should we just have it be a set command with no value?

    return r;

  }else{
    // TODO: Address what happens if ran improperly
    std::cout << "Run again with --help." << endl;
    return 0;
  }


}
