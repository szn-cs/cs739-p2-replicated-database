#include "../include.h"

extern Config config;

// NOTE: I hardcoded the address to work on my local easier. 0.0.0.0 also probably would have worked


pthread_mutex_t Database::data_mutex;
map<string, string> Database::kv_store;

Status DatabaseRPC::get(ServerContext* context, const databaseInterface::GetRequest* request, databaseInterface::GetResponse* response) {
  std::cout << yellow << "DatabaseRPC::get" << reset << std::endl;

  // Check if we are leader by asking consensus thread
  ConsensusRPCWrapperCall* c = new ConsensusRPCWrapperCall(grpc::CreateChannel(config.getLocalIP() + ":8080", grpc::InsecureChannelCredentials()));
  string addr = c->get_leader();

  // Pair to store <value, error_code>. Useful in determining whether a key simply has an empty value or if they key does not exist
  // in the database, allowing us to return an error in the latter case
  std::pair<std::string, int> resp;
  resp.first = "";
  resp.second = 0;

  if(addr == config.getLocalIP()){
    // We are leader, return local value
    resp = Database::Get_KV(request->key());
  }else{
      // We are not leader, contact the db stub of the leader, corresponding to the result of asking the consensus thread for the leader's address
      
      // Psuedo-ish code for what else needs to happen
     
      // DatabaseRPCWrapperCall* leader_c = cluster[message].second; // The database stub
      // value = leader_c.get(key);
      // Database::Set_KV(key, value);

      /*
        Potential issues:
          What happens if the leader is down? Ideally the getleader() call on line 16 would send a heartbeat to the leader, on the conseneus thread
          side, like after it receives the getleader() rpc, it looks at its locally cached leader address and sends a ping to it. If no response,
          it triggers an election, eventually returning the address of the leader to the database thread in the rpc response to getleader()
      */
  }

  if(resp.second == 1){
    response->set_value("");
    response->set_error("DoesNotExist");
  }else{
    response->set_value(resp.first);
  }

  return Status::OK;
}

Status DatabaseRPC::set(ServerContext* context, const databaseInterface::SetRequest* request, databaseInterface::Empty* response) {
  std::cout << yellow << "DatabaseRPC::set" << reset << std::endl;

  // Dummy logic for now, gotta actually trigger a proposal
  Database::Set_KV(request->key(), request->value());

  /*
    We implement this simply by sending a proposal rpc to the leader's consensus thread. The leader will handle reaching consensus, and will
    eventually return the agreed upon value.

    Only issue is if the leader is down. We need to implement timeouts, as shown in psuedo code in the RPCWrapperCall file in the database
    rpc calls code.
  */

  return Status::OK;
}

/**
 * Internal database KV methods 
 * - not accessible by users only quorum participants/nodes.
 * TODO: Is this locking scheme good enough?
 */
std::pair<std::string, int> Database::Get_KV(const string& key) {
  // Returning a pair to indicate if the element does not exist vs if its simply an empty string in the db
  std::pair<std::string, int> res;
  int ret = pthread_mutex_lock(&data_mutex);
  if(ret != 0){
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
  if(ret != 0){
    if (ret == EINVAL) {
        pthread_mutex_init(&data_mutex, NULL);
    }
  }
  kv_store[key] = value;
  pthread_mutex_unlock(&data_mutex);
}

void Database::Delete_KV(const string& key) {
  int ret = pthread_mutex_lock(&data_mutex);
  if(ret != 0){
    if (ret == EINVAL) {
        pthread_mutex_init(&data_mutex, NULL);
    }
  }
  kv_store.erase(key);
  pthread_mutex_unlock(&data_mutex);
}

// map<string, map<int, databaseInterface::LogEntry>> Consensus::Get_Log() {
//   return pax_log;
// }

map<string, string> Database::Get_DB() {
  return kv_store;
}
