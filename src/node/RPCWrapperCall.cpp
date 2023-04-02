#include "./RPCWrapperCall.h"

/* Database RPC wrappers ------------------------------------------------------------- */

DatabaseRPCWrapperCall::DatabaseRPCWrapperCall(std::shared_ptr<grpc::Channel> channel)
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

/* Consensus RPC wrappers ------------------------------------------------------------- */

ConsensusRPCWrapperCall::ConsensusRPCWrapperCall(std::shared_ptr<grpc::Channel> channel)
    : stub(consensusInterface::ConsensusService::NewStub(channel)) {}

std::string ConsensusRPCWrapperCall::propose(const std::string& s) {
  std::cout << yellow << "ConsensusRPCWrapperCall::propose" << reset << std::endl;

  return "default";
}

std::string ConsensusRPCWrapperCall::accept(const std::string& s) {
  std::cout << yellow << "ConsensusRPCWrapperCall::accept" << reset << std::endl;

  return "default";
}

std::string ConsensusRPCWrapperCall::success(const std::string& s) {
  std::cout << yellow << "ConsensusRPCWrapperCall::success" << reset << std::endl;

  return "default";
}

std::string ConsensusRPCWrapperCall::heartbeat(const std::string& s) {
  std::cout << yellow << "ConsensusRPCWrapperCall::heartbeat" << reset << std::endl;

  grpc::ClientContext context;
  consensusInterface::Request request;
  consensusInterface::Response response;

  request.set_value(s);

  grpc::Status status = this->stub->heartbeat(&context, request, &response);

  return (status.ok()) ? "OK" : "RPC failed !";
}
