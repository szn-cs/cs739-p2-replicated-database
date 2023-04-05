#include "./declaration.h"

namespace app {

  template class Endpoint<rpc::call::ConsensusRPCWrapperCall>;  // explicit initiation - prevent linker errors for separate dec & def of template
  template class Endpoint<rpc::call::DatabaseRPCWrapperCall>;   // explicit initiation - prevent linker errors for separate dec & def of template

  std::shared_ptr<std::map<std::string, std::unique_ptr<Node>>> Cluster::memberList = nullptr;
  std::shared_ptr<utility::parse::Config> Cluster::config = nullptr;

  void initializeStaticInstance(std::vector<std::string> addressList, std::shared_ptr<utility::parse::Config> config) {
    Cluster::config = config;

    Cluster::memberList = std::make_shared<std::map<std::string, std::unique_ptr<Node>>>();

    for (std::string& a : addressList)
      Cluster::memberList->insert(std::make_pair(a, std::make_unique<Node>(a)));

    if (config->flag.debug) {  // print list
      // std::map<std::string, std::unique_ptr<Node>>::iterator itr;
      // itr = Cluster::memberList->begin();
      // std::cout << itr->first << std::endl;

      // Transform each config into a address via make_address, inserting each object into the vector.
      std::vector<utility::parse::Address> l;
      std::transform(addressList.begin(), addressList.end(), std::back_inserter(l), utility::parse::make_address);

      // Print nodes.
      std::copy(l.begin(), l.end(), std::ostream_iterator<utility::parse::Address>(std::cout, "\n"));
    }

    // instance initialization taken care of in the class definition.
    // Consensus::instance = std::make_shared<Consensus>();
    // Database::instance = std::make_shared<Database>();
  }

}  // namespace app

namespace app {

  // initialized in class instead.
  // std::shared_ptr<Consensus> Consensus::instance = nullptr;

  void Consensus::consensus_stub_rpc_setup() {
    // if (!Cluster::config.flag.leader)
    //   return;
  }

  void Consensus::initializeProtocol() {
    // sleep(5);
    // for (auto& [key, node] : Cluster::memberList) {
    // node.consensusEndpoint.stub->ping("message");
    // }
  }

  // Methods for adding to log at different points during paxos algorithm
  // when Acceptor receives a proposal for particular key and round
  void Consensus::Set_Log(const string& key, int round) {
    pthread_mutex_lock(&log_mutex);
    pax_log[key][round];
    pthread_mutex_unlock(&log_mutex);
  }

  // update proposal server
  void Consensus::Set_Log(const string& key, int round, int p_server) {
    pthread_mutex_lock(&log_mutex);
    pax_log[key][round].set_p_server_id(p_server);
    pthread_mutex_unlock(&log_mutex);
  }

  // Update the acceptance info for the given key and round.
  void Consensus::Set_Log(const string& key, int round, int a_server, databaseInterface::Operation op, string value) {
    pthread_mutex_lock(&log_mutex);
    pax_log[key][round].set_a_server_id(a_server);
    pax_log[key][round].set_op(op);
    pax_log[key][round].set_accepted_value(value);
    pthread_mutex_unlock(&log_mutex);
  }

  map<string, map<int, databaseInterface::LogEntry>> Consensus::Get_Log() {
    map<string, map<int, databaseInterface::LogEntry>> c;
    return c;
  }

  // Find the highest proposal number seen for the given key
  pair<string, int> Consensus::Find_Max_Proposal(const string& key, int round) {
    map<string, map<int, databaseInterface::LogEntry>> pax_log = Consensus::Get_Log();

    //pthread_mutex_lock(&log_mutex);
    int max_proposal = round;
    for (const auto& entry : pax_log[key]) {
      if (entry.first > max_proposal) {
        max_proposal = entry.first;
      }
    }
    //pthread_mutex_unlock(&log_mutex);

    if (max_proposal == round)
      return make_pair("", 0);

    return make_pair(pax_log[key][max_proposal].accepted_value(), max_proposal);
  }

  string Consensus::readFromDisk(string path) {
    std::ifstream file(path);
    std::string value;
    std::getline(file, value);

    return value;
  }

  void Consensus::writeToDisk(string path, string value) {
    ofstream file2(path, std::ios::trunc);  // open the file for writing, truncate existing content
    file2 << value;                         // write the new content to the file
    file2.close();
  }

  std::string Consensus::GetLeader() {
    // Threadsafe read of leader address
    pthread_mutex_lock(&leader_mutex);
    std::string leader = leader_;
    pthread_mutex_unlock(&leader_mutex);
    return leader;
  }

}  // namespace app

namespace app {

  // initialized in class instead
  // std::shared_ptr<Database> Database::instance = nullptr;

  /**
   * Internal database KV methods
   * - not accessible by users only quorum participants/nodes.
   * TODO: Is this locking scheme good enough?
   */
  string Database::Get_KV(const string& key) {
    pthread_mutex_lock(&data_mutex);
    auto i = kv_store.find(key);
    if (i == kv_store.end()) {
      return NULL;
    }
    pthread_mutex_unlock(&data_mutex);
    return i->second;
  }

  void Database::Set_KV(const string& key, string& value) {
    pthread_mutex_lock(&data_mutex);
    kv_store[key] = value;
    pthread_mutex_unlock(&data_mutex);
  }

  void Database::Delete_KV(const string& key) {
    pthread_mutex_lock(&data_mutex);
    kv_store.erase(key);
    pthread_mutex_unlock(&data_mutex);
  }

  map<string, string> Database::Get_DB() {
    return kv_store;
  }

}  // namespace app