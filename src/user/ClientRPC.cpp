
#include "./ClientRPC.h"

// implement class constructor
ClientRPC::ClientRPC(std::shared_ptr<Channel> channel) : stub(databaseInterface::DatabaseService::NewStub(channel)) {}

std::string ClientRPC::get(const std::string& s) {
  std::cout << yellow << "ClientRPC::get" << reset << std::endl;

  grpc::ClientContext context;
  databaseInterface::Request request;
  databaseInterface::Response response;

  request.set_s(s);

  grpc::Status status = this->stub->get(&context, request, &response);

  return (status.ok()) ? response.m() : "RPC failed !";
}

std::string ClientRPC::set(const std::string& s) {
  std::cout << yellow << "ClientRPC::set" << reset << std::endl;

  grpc::ClientContext context;
  databaseInterface::Request request;
  databaseInterface::Response response;

  request.set_s(s);

  grpc::Status status = this->stub->set(&context, request, &response);

  return (status.ok()) ? response.m() : "RPC failed !";
}
