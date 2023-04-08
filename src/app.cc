#include "./declaration.h"

namespace app {

  template class Endpoint<rpc::call::ConsensusRPCWrapperCall>;  // explicit initiation - prevent linker errors for separate dec & def of template
  template class Endpoint<rpc::call::DatabaseRPCWrapperCall>;   // explicit initiation - prevent linker errors for separate dec & def of template

  std::shared_ptr<std::map<std::string, std::shared_ptr<Node>>> Cluster::memberList = nullptr;
  std::shared_ptr<utility::parse::Config> Cluster::config = nullptr;
  std::shared_ptr<Node> Cluster::currentNode = nullptr;

  void initializeStaticInstance(std::vector<std::string> addressList, std::shared_ptr<utility::parse::Config> config) {
    Cluster::config = config;

    Cluster::memberList = std::make_shared<std::map<std::string, std::shared_ptr<Node>>>();
    for (std::string& a : addressList)
      Cluster::memberList->insert(std::make_pair(a, std::make_shared<Node>(a)));

    // current node's details
    utility::parse::Address addressConsensus = Cluster::config->getAddress<app::Service::Consensus>();  // addresses as IDs follow the consensus port
    utility::parse::Address addressDatabase = Cluster::config->getAddress<app::Service::Database>();    // addresses as IDs follow the consensus port
    auto iterator = Cluster::memberList->find(addressConsensus.toString());
    if (iterator == Cluster::memberList->end()) {  // not found
      Cluster::currentNode = std::make_shared<Node>(addressConsensus, addressDatabase);
      Cluster::memberList->insert(std::make_pair(addressConsensus.toString(), Cluster::currentNode));
    } else {
      Cluster::currentNode = iterator->second;
    }

    Cluster::leader = "";

    if (config->flag.debug) {  // print list
      // std::map<std::string, std::shared_ptr<Node>>::iterator itr;
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

  Status Consensus::coordinate() {
    if (Cluster::config->flag.leader) {
      // Can use self to indicate if this replica is a leader, an address otherwise
      Cluster::config->flag.leader = "self";
      return Status::OK;
    } else {
      // Must send get_coordinator requests to other stubs
      // Because this is a call not going explicitly to leader, need to track which
      // nodes are live
      std::set<std::string> leaders;
      std::set<std::string> live_replicas;
      for (const auto& [key, node] : *(Cluster::memberList)) {
        std::pair<Status, std::string> res = node->consensusEndpoint.stub->get_leader();
        if (res.first.ok()) {  // Replica is up
          live_replicas.insert(key);
          if (!res.second.empty()) {  // Replica knows the current leader
            leaders.insert(res.second);
          }
        }
      }

      // leaders.size() will be 1 or 0, unless consensus issues
      // If 0, trigger an election
      // Check if leader is alive, if not return a non ok status to trigger an election
      if (leaders.size() == 1 && live_replicas.find(*leaders.begin()) == live_replicas.end()) {
        pthread_mutex_lock(&(Cluster::leader_mutex));
        Cluster::leader = *leaders.begin();
        pthread_mutex_unlock(&(Cluster::leader_mutex));
      } else {
        // Send an election request to ourself
        Cluster::currentNode->consensusEndpoint.stub->trigger_election();
      }

      return Status::OK;
    }
  }

  void Consensus::broadcastPeriodicPing() {
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
  void Consensus::Set_Log(const string& key, int round, int a_server, database_interface::Operation op, string value) {
    pthread_mutex_lock(&log_mutex);
    pax_log[key][round].set_a_server_id(a_server);
    pax_log[key][round].set_op(op);
    pax_log[key][round].set_accepted_value(value);
    pthread_mutex_unlock(&log_mutex);
  }

  map<string, map<int, database_interface::LogEntry>> Consensus::Get_Log() {
    map<string, map<int, database_interface::LogEntry>> c;
    return c;
  }

  // Find the highest proposal number seen for the given key
  pair<string, int> Consensus::Find_Max_Proposal(const string& key, int round) {
    map<string, map<int, database_interface::LogEntry>> pax_log = Consensus::Get_Log();

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
    // TODO: This is not a proper usage of locks
    // Threadsafe read of leader address
    pthread_mutex_lock(&Cluster::leader_mutex);
    std::string leader = Cluster::leader;
    pthread_mutex_unlock(&Cluster::leader_mutex);

    return leader;
  }

  // std::string Consensus::SetLeader() {
  //   // TODO: This is not a proper usage of locks
  //   // Do we even need to lock a read like this???

  //   // Threadsafe read of leader address
  //   //pthread_mutex_lock(&Cluster::leader_mutex);
  //   //std::string leader = Cluster::leader;
  //   //pthread_mutex_unlock(&Cluster::leader_mutex);

  //   return Cluster::leader;
  // }


  template <typename Req> // Either ElectLeader request or Put request or Delete request if we implement delete separate than Put
  Status Consensus::AttemptConsensus(const Req& r){
    std::string key = r.key();
    std::string value = r.value();
    
    // Get current round
    map<string, map<int, database_interface::LogEntry>> pax_log = Consensus::Get_Log();
    int round = pax_log[key].size();

    // Set proposal id to 1, first proposal we are making from this replica
    int propose_id = 1;

    // Ping servers, figure out who's alive
    std::vector<Node*> live_nodes;
    for (const auto& [key, node] : *(Cluster::memberList)) {
      Status res = node->consensusEndpoint.stub->ping(node->consensusEndpoint.address, 8000); // 8000 is consensus port
      if(res.ok()){
        live_nodes.push_back(node.get());
      }
    }

    // Do we have enough for quorum?
    // TODO:: Non-hardcoded value for quorum, should be stored as a data member for Consensus class and
    // instantiated during initialization
    int num_live_acceptors = live_nodes.size();
    if(num_live_acceptors <= NUM_REPLICAS / 2){
      return Status(grpc::StatusCode::ABORTED, "Not enough live servers for quorum.");
    }

    // 1. Entering the proposal stage

    // Preparing the request
    Request prepare_request;
    prepare_request.set_key(key);
    prepare_request.set_round(round);
    prepare_request.set_pserver_id(propose_id);

    // Sending the request to all live nodes, tracking if a value has already been accepted for
    // our key
    int num_accepted_proposals = 0;
    int accepted_id = 0;
    std::string accepted_value;
    consensus_interface::Operation accepted_op = consensus_interface::Operation::NOT_SET;

    // const auto n = Cluster::leader;
    // Status response = n->consensusEndpoint.stub->propose(prepare_request);

    //string leader = GetLeader();

    for(const Node* n : live_nodes){
      Response response = n->consensusEndpoint.stub->propose(prepare_request);
      if(response.status() == Status_Types::OK){
        num_accepted_proposals++;
        if(response.aserver_id() > accepted_id){
          accepted_id = response.aserver_id();
          accepted_value = response.value();
          accepted_op = response.op();
        }
      }
    }
    std::cout << termcolor::cyan << num_accepted_proposals << " servers accepted our proposal.";

    // Check if we still have a quorum
    if(num_accepted_proposals <= NUM_REPLICAS / 2){
      // A node must have died between our first ping and here
      return Status(grpc::StatusCode::ABORTED, "Failed to achieve quorum.");
    }

    // 2. Entering the accept stage

    // int num_final_acceptances = 0;

    // Request accept_request;
    // accept_request.set_key(key);
    // accept_request.set_round(round);
    // accept_request.set_pserver_id(propose_id);

    // for(const Node* n : live_nodes){
    //   Response response = n->consensusEndpoint.stub->accept(prepare_request);
    //   if(response.status() == Status_Types::OK){
    //     num_final_acceptances++;
    //     if(response.aserver_id() > accepted_id){
    //       accepted_id = response.aserver_id();
    //       accepted_value = response.value();
    //       accepted_op = response.op();
    //     }
    //   }
    // }

    // if(num_final_acceptances <= NUM_REPLICAS / 2){
    //   // A node must have died between our first ping and here
    //   return Status(grpc::StatusCode::ABORTED, "Failed to achieve quorum.");
    // } else {

      //commit

    //}

    return Status::OK;

  }

}  // namespace app

namespace app {

  std::string Cluster::leader;
  pthread_mutex_t Cluster::leader_mutex;

  // initialized in class instead
  // std::shared_ptr<Database> Database::instance = nullptr;

  /**
   * Internal database KV methods
   * - not accessible by users only quorum participants/nodes.
   * TODO: Is this locking scheme good enough?
   */
  std::pair<std::string, int> Database::Get_KV(const string& key) {
    // Returning a pair to indicate if the element does not exist vs if its simply an empty string in the db
    std::pair<std::string, int> res;
    int ret = pthread_mutex_lock(&data_mutex);
    if (ret != 0) {
      if (ret == EINVAL) {
        pthread_mutex_init(&data_mutex, NULL);
      }
    }
    auto i = kv_store.find(key);
    if (i == kv_store.end()) {
      res.first = "";
      res.second = 1;
      return res;
    }
    pthread_mutex_unlock(&data_mutex);
    res.first = i->second;
    res.second = 0;
    return res;
  }

  void Database::Set_KV(const string& key, const string& value) {
    int ret = pthread_mutex_lock(&data_mutex);
    if (ret != 0) {
      if (ret == EINVAL) {
        pthread_mutex_init(&data_mutex, NULL);
      }
    }
    kv_store[key] = value;
    pthread_mutex_unlock(&data_mutex);
  }

  void Database::Delete_KV(const string& key) {
    int ret = pthread_mutex_lock(&data_mutex);
    if (ret != 0) {
      if (ret == EINVAL) {
        pthread_mutex_init(&data_mutex, NULL);
      }
    }
    kv_store.erase(key);
    pthread_mutex_unlock(&data_mutex);
  }

  map<string, string> Database::Get_DB() {
    return kv_store;
  }

}  // namespace app