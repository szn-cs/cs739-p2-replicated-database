#include "./declaration.h"

int user_entrypoint(std::shared_ptr<utility::parse::Config> config);

/**
 * initialize configurations, run RPC servers, and start consensus coordination
*/
int main(int argc, char* argv[]) {
  cout << termcolor::grey << utility::getClockTime() << termcolor::reset << endl;

  struct stat info;
  std::shared_ptr<utility::parse::Config> config = std::make_shared<utility::parse::Config>();

  // parse options from different sources
  utility::parse::parse_options(argc, argv, config);

  /** pick Mode of opeartion: either run distributed database or run the user testing part. */
  // for (utility::parse::Mode m : config->mode)
  //   std::cout << m << std::endl;

  switch (config->mode) {
    case utility::parse::Mode::USER:
      return user_entrypoint(config);
      break;
    case utility::parse::Mode::NODE:
    default:
      // continue
      break;
  }

  // Initialize Cluster data & Node instances
  app::initializeStaticInstance(config->cluster, config);

  // handle database directory TODO:
  fs::create_directories(fs::absolute(config->database_directory));  // create database direcotry directory if doesn't exist

  // RPC services on separate threads
  utility::parse::Address a1 = config->getAddress<app::Service::Consensus>();
  std::thread t1(utility::server::run_gRPC_server<rpc::ConsensusRPC>, a1);
  utility::parse::Address a2 = config->getAddress<app::Service::Database>();
  std::thread t2(utility::server::run_gRPC_server<rpc::DatabaseRPC>, a2);
  std::cout << termcolor::blue << "⚡ Consensus service: " << a1.address + ":" + std::to_string(a1.port) << " | Database service: " << a2.address + ":" + std::to_string(a2.port) << termcolor::reset << std::endl;
  // ping broadcasting thread (every randomly picked interval)
  std::thread t3(app::Consensus::initializeProtocol);

  // start cluster coordination
  app::Consensus::consensus_stub_rpc_setup();

  t1.join();
  t2.join();
  t3.join();

  cout << termcolor::grey << utility::getClockTime() << termcolor::reset << endl;
  return 0;
}

/**
 * Handle configurations: 
 * 1. service database RPC endpoints/addresses
 * 2. coordination settings for testing
*/
int user_entrypoint(std::shared_ptr<utility::parse::Config> config) {
  int r = 0;
  string db_address = "127.0.1.1:9000";  // target address & port to send grpc requests to.

  rpc::call::DatabaseRPCWrapperCall* c = new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

  c->set("1", "1");

  string message = c->get("1");

  std::cout << message << std::endl;

  return r;
}
