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

  if (config->flag.debug)
    cout << termcolor::grey << "Using config file at: " << config->config << termcolor::reset << endl;

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
  if (config->flag.coordinate)
    app::Consensus::instance->coordinate();

  t1.join();
  t2.join();
  //t3.join();

  cout << termcolor::grey << utility::getClockTime() << "Node exited" << termcolor::reset << endl;
  return 0;
}

/** User execution for testing the RPC servers */
int user_entrypoint(std::shared_ptr<utility::parse::Config> config, boost::program_options::variables_map& variables) {
  //   ./target/app --mode user --command set --key k1 --value v1 --target 0.0.0.0:8002
  if (!variables.count("command"))
    throw "`command` argument is required";

  // if command exists
  auto command = variables["command"].as<std::string>();
  auto target = variables["target"].as<std::string>();
  auto key = variables["key"].as<std::string>();
  auto value = variables["value"].as<std::string>();

  std::cout << "arguments provided: " << command + " " + key + " " + value + " " + target << std::endl;
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

  } else if (command == "test_leader_c") {
    // TODO: Address what happens if ran improperly
    // std::cout << "Run again with --help." << endl;

    // TODO: This is just for testing, get rid of this stuff
    string db_address = "127.0.1.1:8000";
    rpc::call::ConsensusRPCWrapperCall* leader_conn = new rpc::call::ConsensusRPCWrapperCall(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

    // Test ping() when called on leader
    Status r1 = leader_conn->ping();
    if (r1.ok()) {
      std::cout << termcolor::cyan << "Ping works" << reset << std::endl;
    } else {
      std::cout << termcolor::red << "Ping broken" << reset << std::endl;
    }

    // Test get_leader() when called on leader
    std::pair<Status, string> resp = leader_conn->get_leader();
    std::cout << termcolor::cyan << "get_leader() error code " << resp.first.error_code() << " returned leader address: " << resp.second << reset << std::endl;

  } else if (command == "test_leader_db") {
    string db_address = "127.0.1.1:9000";
    rpc::call::DatabaseRPCWrapperCall* leader_conn = new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

    Status s = leader_conn->set("1", "v1");
    std::cout << termcolor::cyan << "set() error code " << s.error_code() << reset << std::endl;

    std::string g = leader_conn->get("1");
    std::cout << termcolor::cyan << "value returned was " << g << reset << std::endl;

  } else if (command == "test_non_leader_c") {
    string db_address = "127.0.1.1:8001";

    rpc::call::ConsensusRPCWrapperCall* replica_con = new rpc::call::ConsensusRPCWrapperCall(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

    // Test ping() when called on non leader
    Status r1 = replica_con->ping();
    if (r1.ok()) {
      std::cout << termcolor::cyan << "Ping works" << reset << std::endl;
    } else {
      std::cout << termcolor::red << "Ping broken" << reset << std::endl;
    }

    // Test get_leader() when called on non leader
    std::pair<Status, string> resp = replica_con->get_leader();
    std::cout << termcolor::cyan << "get_leader() error code " << resp.first.error_code() << " returned leader address: " << resp.second << reset << std::endl;

  } else if (command == "test_non_leader_db") {
    //   string db_address = "127.0.1.1:9000";
    //   rpc::call::DatabaseRPCWrapperCall* leader_conn = new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

    //   string non_leader_addr = "127.0.1.1:9001";
    //   rpc::call::DatabaseRPCWrapperCall* replica_conn = new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(non_leader_addr, grpc::InsecureChannelCredentials()));

    //   Status s = leader_conn->set("1", "v1");
    //   std::cout << termcolor::cyan << "set() error code " << s.error_code() << reset << std::endl;

    //   std::string g = replica_conn->get("1");
    //   std::cout << termcolor::cyan << "value returned was " << g << reset << std::endl;
    // } else if (command == "test_5_random_ops") {
    //   std::vector<rpc::call::DatabaseRPCWrapperCall*> db_addrs;
    //   db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9000", grpc::InsecureChannelCredentials())));
    //   db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9001", grpc::InsecureChannelCredentials())));
    //   db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9002", grpc::InsecureChannelCredentials())));
    //   db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9003", grpc::InsecureChannelCredentials())));
    //   db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9004", grpc::InsecureChannelCredentials())));

    //   std::vector<std::string> str_addrs;
    //   str_addrs.push_back("127.0.1.1:9000");
    //   str_addrs.push_back("127.0.1.1:9001");
    //   str_addrs.push_back("127.0.1.1:9002");
    //   str_addrs.push_back("127.0.1.1:9003");
    //   str_addrs.push_back("127.0.1.1:9004");

    string address_random = "127.0.1.1:8000";
    rpc::call::ConsensusRPCWrapperCall* c = new rpc::call::ConsensusRPCWrapperCall(grpc::CreateChannel(address_random, grpc::InsecureChannelCredentials()));
    std::pair<Status, std::string> res = c->get_leader();
    if (!res.first.ok())
      throw std::runtime_error("RPC FAILURE");

    string address_leader = res.second;
    std::cout << "Leader is: " << address_leader << std::endl;

    std::vector<rpc::call::DatabaseRPCWrapperCall*> db_addrs;
    std::vector<std::string> str_addrs;
    for (const auto& [key, node] : *(app::Cluster::memberList)) {
      db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel(node->databaseEndpoint.address, grpc::InsecureChannelCredentials())));
      str_addrs.push_back(node->databaseEndpoint.address);
    }

    std::vector<std::string> keys;
    keys.push_back("a");
    keys.push_back("b");
    keys.push_back("c");
    keys.push_back("d");
    keys.push_back("e");
    keys.push_back("f");
    keys.push_back("g");
    keys.push_back("h");
    keys.push_back("i");
    keys.push_back("j");

    int num_ops = 100;

    srand(123);

    char alpha[26] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    for (int i = 0; i < num_ops; i++) {
      string result = "";
      for (int i = 0; i < 5; i++) {
        result = result + alpha[rand() % 26];
      }
      int replica_idx = rand() % db_addrs.size();
      int key_idx = rand() % keys.size();
      Status s = db_addrs[replica_idx]->set(keys[key_idx], result);
      if (!s.ok()) {
        std::cout << reset << red << "Set(" << keys[key_idx] << ", " << result << ") failed" << reset << endl;
      }
    }

    std::map<std::string, std::vector<std::string>> results;
    for (unsigned long i = 0; i < db_addrs.size(); i++) {
      google::protobuf::Map<string, string> res = db_addrs[i]->get_db();
      for (auto& kb : res) {
        results[str_addrs[i]].push_back(kb.second);
      }
    }

    for (const auto& [key, value] : results) {
      std::cout << cyan << key << ": " << reset;
      copy(value.begin(), value.end(), ostream_iterator<std::string>(std::cout, " "));
      std::cout << reset << endl;
    }
  } else if (command == "test_1000_random_ops") {
    std::vector<rpc::call::DatabaseRPCWrapperCall*> db_addrs;
    db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9000", grpc::InsecureChannelCredentials())));
    db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9001", grpc::InsecureChannelCredentials())));
    db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9002", grpc::InsecureChannelCredentials())));
    db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9003", grpc::InsecureChannelCredentials())));
    db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9004", grpc::InsecureChannelCredentials())));
    db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9005", grpc::InsecureChannelCredentials())));
    db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9006", grpc::InsecureChannelCredentials())));
    db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9007", grpc::InsecureChannelCredentials())));
    db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9008", grpc::InsecureChannelCredentials())));
    db_addrs.push_back(new rpc::call::DatabaseRPCWrapperCall(grpc::CreateChannel("127.0.1.1:9009", grpc::InsecureChannelCredentials())));

    std::vector<std::string> str_addrs;
    str_addrs.push_back("127.0.1.1:9000");
    str_addrs.push_back("127.0.1.1:9001");
    str_addrs.push_back("127.0.1.1:9002");
    str_addrs.push_back("127.0.1.1:9003");
    str_addrs.push_back("127.0.1.1:9004");
    str_addrs.push_back("127.0.1.1:9005");
    str_addrs.push_back("127.0.1.1:9006");
    str_addrs.push_back("127.0.1.1:9007");
    str_addrs.push_back("127.0.1.1:9008");
    str_addrs.push_back("127.0.1.1:9009");

    std::vector<std::string> keys;
    keys.push_back("a");
    keys.push_back("b");
    keys.push_back("c");
    keys.push_back("d");
    keys.push_back("e");
    keys.push_back("f");
    keys.push_back("g");
    keys.push_back("h");
    keys.push_back("i");
    keys.push_back("j");

    int num_ops = 1000;

    srand(123);

    char alpha[26] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    for (int i = 0; i < num_ops; i++) {
      string result = "";
      for (int i = 0; i < 5; i++) {
        result = result + alpha[rand() % 26];
      }
      int replica_idx = rand() % db_addrs.size();
      int key_idx = rand() % keys.size();
      Status s = db_addrs[replica_idx]->set(keys[key_idx], result);
      if (!s.ok()) {
        std::cout << reset << red << "Set(" << keys[key_idx] << ", " << result << ") failed" << reset << endl;
      }
    }

    std::map<std::string, std::vector<std::string>> results;
    for (int i = 0; i < (int)str_addrs.size(); i++) {
      google::protobuf::Map<string, string> r = db_addrs[i]->get_db();
      results[str_addrs[i]];
      for (std::string key : keys) {
        results[str_addrs[i]].push_back(r[key]);
      }
    }

    for (const auto& [key, value] : results) {
      std::cout << cyan << key << ": " << reset;
      copy(value.begin(), value.end(), ostream_iterator<std::string>(std::cout, " "));
      std::cout << reset << endl;
    }
  } else if (command == "test_count") {
    cout << yellow << "incoming request count " << reset << endl;

    std::vector<rpc::call::ConsensusRPCWrapperCall*> addrs;
    std::vector<std::string> str_addrs;
    for (const auto& [key, node] : *(app::Cluster::memberList)) {
      addrs.push_back(new rpc::call::ConsensusRPCWrapperCall(grpc::CreateChannel(node->databaseEndpoint.address, grpc::InsecureChannelCredentials())));
      str_addrs.push_back(node->databaseEndpoint.address);
    }

    std::map<std::string, std::vector<std::string>> results;

    for (unsigned long i = 0; i < addrs.size(); i++) {
      std::tuple<Status, std::map<std::string, int>, std::map<std::string, int>> res = addrs[i]->get_stats();
      auto [status, incount, outcount] = res;
      if (!status.ok())
        cout << red << "RPC failure: get_stats failed" << reset << endl;

      for (auto const& x : incount) {
        std::cout << x.first  // string (key)
                  << ':'
                  << x.second  // string's value
                  << std::endl;
      }

      cout << yellow << "outgoing requests count" << reset << endl;
      for (auto const& x : outcount) {
        std::cout << x.first  // string (key)
                  << ':'
                  << x.second  // string's value
                  << std::endl;
      }
    }
  }

  return 0;
}