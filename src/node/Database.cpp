#include "Database.h"

Status DatabaseRPC::get(ServerContext* context, const databaseInterface::Request* request, databaseInterface::Response* response) {
  std::cout << yellow << "DatabaseRPC::get" << reset << std::endl;

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

DatabaseClientRPC::DatabaseClientRPC(std::shared_ptr<Channel> channel)
    : stub(databaseInterface::DatabaseService::NewStub(channel)) {}

std::string DatabaseClientRPC::get(const std::string& s) {
  std::cout << yellow << "DatabaseClientRPC::get" << reset << std::endl;

  return "default";
}

std::string DatabaseClientRPC::set(const std::string& s) {
  std::cout << yellow << "DatabaseClientRPC::set" << reset << std::endl;

  return "default";
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

map<string, map<int, databaseInterface::LogEntry>> Database::Get_Log() {
  return pax_log;
}

map<string, string> Database::Get_DB() {
  return kv_store;
}

// Methods for adding to log at different points during paxos algorithm
void Database::Set_Log(const string& key, int round) {
  pthread_mutex_lock(&log_mutex);
  pax_log[key][round];
  pthread_mutex_unlock(&log_mutex);
}

void Database::Set_Log(const string& key, int round, int p_server) {
  pthread_mutex_lock(&log_mutex);
  pax_log[key][round].set_p_server_id(p_server);
  pthread_mutex_unlock(&log_mutex);
}

void Database::Set_Log(const string& key, int round, int a_server, databaseInterface::Operation op, string value) {
  pthread_mutex_lock(&log_mutex);
  pax_log[key][round].set_a_server_id(a_server);
  pax_log[key][round].set_op(op);
  pax_log[key][round].set_accepted_value(value);
  pthread_mutex_unlock(&log_mutex);
}
