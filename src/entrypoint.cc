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
  Status s = app::Consensus::instance->coordinate();
  if(!s.ok()){
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << red 
      << "Unable to initialize node because " << s.error_message() << reset << std::endl;
  }

  t1.join();
  t2.join();
  //t3.join();

  cout << termcolor::grey << utility::getClockTime() << "Node exited" << termcolor::reset << endl;
  return 0;
}

/** User execution for testing the RPC servers */
int user_entrypoint(std::shared_ptr<utility::parse::Config> config, boost::program_options::variables_map& variables) {
  // TODO: parse options for sending key value pair to address of cluster member for testing purposes through terminal.

  //   ./target/app --mode user --command set --key k1 --value v1 --target 0.0.0.0:8002
  if (variables.count("command")) {  // if command exists
    auto command = variables["command"].as<std::string>();
    auto target = variables["target"].as<std::string>();
    auto key = variables["key"].as<std::string>();
    auto value = variables["value"].as<std::string>();

    std::cout << command + " " + key + " " + value + " send to " + target << std::endl;
    // TODO: create RPC requests to modify the database.
    //string db_address = "127.0.1.1:9000";  // target address & port to send grpc requests to.

    rpc::call::DatabaseRPCWrapperCall* c = new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
    // NOTE: above statement equivalent to be equivalent to if proper configs added
    // auto iterator = app::Cluster::memberList->find(target);
    // std::shared_ptr<app::Node> targetNode = iterator->second;
    // std::shared_ptr<rpc::call::DatabaseRPCWrapperCall> c_equivalent = targetNode->databaseEndpoint.stub;

    if (command == "set") {
      rpc::call::DatabaseRPCWrapperCall* c = new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
      c->set(key, value);

    } else if (command == "get") {
      rpc::call::DatabaseRPCWrapperCall* c = new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
      string message = c->get(key);
      std::cout << message << std::endl;


    } else if(command == "test_leader_c"){
      // TODO: Address what happens if ran improperly
      // std::cout << "Run again with --help." << endl;

      // TODO: This is just for testing, get rid of this stuff
      string db_address = "127.0.1.1:8000";
      rpc::call::ConsensusRPCWrapperCall* leader_conn = new rpc::call::ConsensusRPCWrapperCall(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

      // Test ping() when called on leader
      Status r1 = leader_conn->ping();
      if(r1.ok()){
        std::cout << termcolor::cyan << "Ping works" << reset << std::endl;
      }else{
        std::cout << termcolor::red << "Ping broken" << reset << std::endl;
      }

      // Test get_leader() when called on leader
      std::pair<Status, string> resp = leader_conn->get_leader();
      std::cout << termcolor::cyan << "get_leader() error code " << resp.first.error_code() <<  " returned leader address: " << resp.second << reset << std::endl;


    } else if(command == "test_leader_db"){
      string db_address = "127.0.1.1:9000";
      rpc::call::DatabaseRPCWrapperCall* leader_conn = new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

      Status s = leader_conn->set("1", "v1");
      std::cout << termcolor::cyan << "set() error code " << s.error_code() << reset << std::endl;

      std::string g = leader_conn->get("1");
      std::cout << termcolor::cyan << "value returned was " << g << reset << std::endl;


    } else if(command == "test_non_leader_c"){
      string db_address = "127.0.1.1:8001";

      rpc::call::ConsensusRPCWrapperCall* replica_con = new rpc::call::ConsensusRPCWrapperCall(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

      // Test ping() when called on non leader
      Status r1 = replica_con->ping();
      if(r1.ok()){
        std::cout << termcolor::cyan << "Ping works" << reset << std::endl;
      }else{
        std::cout << termcolor::red << "Ping broken" << reset << std::endl;
      }

      // Test get_leader() when called on non leader
      std::pair<Status, string> resp = replica_con->get_leader();
      std::cout << termcolor::cyan << "get_leader() error code " << resp.first.error_code() <<  " returned leader address: " << resp.second << reset << std::endl;
    
    } else if(command == "test_non_leader_db"){
      string db_address = "127.0.1.1:9000";
      rpc::call::DatabaseRPCWrapperCall* leader_conn = new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

      string non_leader_addr = "127.0.1.1:9001";
      rpc::call::DatabaseRPCWrapperCall* replica_conn = new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(non_leader_addr, grpc::InsecureChannelCredentials()));
      
      Status s = leader_conn->set("1", "v1");
      std::cout << termcolor::cyan << "set() error code " << s.error_code() << reset << std::endl;

      std::string g = replica_conn->get("1");
      std::cout << termcolor::cyan << "value returned was " << g << reset << std::endl;
    }
  } 

  return 0;
}
