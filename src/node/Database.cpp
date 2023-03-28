#include "Database.h"

DatabaseClientRPC::DatabaseClientRPC(std::shared_ptr<Channel> channel)
    : stub(databaseInterface::DatabaseService::NewStub(channel)) {}

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

std::string DatabaseClientRPC::get(const std::string& s) {
  std::cout << yellow << "DatabaseClientRPC::get" << reset << std::endl;

  return "default";
}

std::string DatabaseClientRPC::set(const std::string& s) {
  std::cout << yellow << "DatabaseClientRPC::set" << reset << std::endl;

  return "default";
}
