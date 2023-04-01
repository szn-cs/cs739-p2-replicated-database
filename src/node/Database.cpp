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

DatabaseRPCWrapperCall::DatabaseRPCWrapperCall(std::shared_ptr<Channel> channel)
    : stub(databaseInterface::DatabaseService::NewStub(channel)) {}

std::string DatabaseRPCWrapperCall::get(const std::string& s) {
  std::cout << yellow << "DatabaseRPCWrapperCall::get" << reset << std::endl;

  grpc::ClientContext context;
  databaseInterface::Request request;
  databaseInterface::Response response;

  request.set_s(s);

  grpc::Status status = this->stub->get(&context, request, &response);

  return (status.ok()) ? response.m() : "RPC failed !";
}

std::string DatabaseRPCWrapperCall::set(const std::string& s) {
  std::cout << yellow << "DatabaseRPCWrapperCall::set" << reset << std::endl;

  grpc::ClientContext context;
  databaseInterface::Request request;
  databaseInterface::Response response;

  request.set_s(s);

  grpc::Status status = this->stub->set(&context, request, &response);

  return (status.ok()) ? response.m() : "RPC failed !";
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
