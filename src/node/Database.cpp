#include "../include.h"

extern Config config;

Status DatabaseRPC::get(ServerContext* context, const databaseInterface::Request* request, databaseInterface::Response* response) {
  std::cout << yellow << "DatabaseRPC::get" << reset << std::endl;

  // TODO: We need to share the map of stubs with both the consensus thread and the db thread, so that the db thread can communicate
  // with other db threads. Idk the best way to do that given our current code setup, i.e. not using constructors for the most part.

  // Check if we are leader by asking consensus thread or by referencing a shared data structure, if we so choose
  // If so, return Database::GetKV(request->s())

  // If not, contact the db stub of the leader, corresponding to the result of asking the consensus thread for the leader's address

  response->set_m("server recieved message: " + request->s());
  return Status::OK;
}

Status DatabaseRPC::set(ServerContext* context, const databaseInterface::Request* request, databaseInterface::Response* response) {
  std::cout << yellow << "DatabaseRPC::set" << reset << std::endl;

  // loop over the nodes the try to get a majority consensus
  // grpc req through
  // call stub_Consensus corresponding to ConsensusService

  return Status::OK;
}

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

// map<string, map<int, databaseInterface::LogEntry>> Consensus::Get_Log() {
//   return pax_log;
// }

map<string, string> Database::Get_DB() {
  return kv_store;
}
