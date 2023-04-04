#include "../include.h"

/* Database RPC wrappers ------------------------------------------------------------- */

DatabaseRPCWrapperCall::DatabaseRPCWrapperCall(std::shared_ptr<grpc::Channel> channel)
    : stub(databaseInterface::DatabaseService::NewStub(channel)) {}

std::string DatabaseRPCWrapperCall::get(const std::string& s) {
  std::cout << yellow << "DatabaseRPCWrapperCall::get" << reset << std::endl;

  grpc::ClientContext context;
  // auto deadline =
  //     std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
  // context.set_deadline(deadline);
  databaseInterface::GetRequest request;
  databaseInterface::GetResponse response;

  request.set_key(s);

  grpc::Status status = this->stub->get(&context, request, &response);

  // if (status.error_code() == grpc::StatusCode::UNAVAILABLE ||
  //     status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED) {
  //   // Elect new leader
  //   // Resend request to new leader
  // }


  return (status.ok()) ? response.value() : "RPC failed !";
}

std::string DatabaseRPCWrapperCall::set(const std::string& key, const std::string& value) {
  std::cout << yellow << "DatabaseRPCWrapperCall::set" << reset << std::endl;

  grpc::ClientContext context;
  // auto deadline =
  //     std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
  // context.set_deadline(deadline);
  databaseInterface::SetRequest request;
  databaseInterface::Empty response;

  request.set_key(key);
  request.set_value(value);

  grpc::Status status = this->stub->set(&context, request, &response);

  // if (status.error_code() == grpc::StatusCode::UNAVAILABLE ||
  //     status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED) {
  //   // Elect new leader
  //   // Resend request to new leader
  // }

  return (status.ok()) ? "" : "RPC failed !";
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


std::string ConsensusRPCWrapperCall::get_leader() {
  std::cout << yellow << "GetLeader::success" << reset << std::endl;
  grpc::ClientContext context;
  consensusInterface::Empty request;
  consensusInterface::GetLeaderResponse response;

  grpc::Status status = this->stub->get_leader(&context, request, &response);

  return (status.ok()) ? response.leader() : "RPC failed !";
  return "default";
}

std::string ConsensusRPCWrapperCall::ping(const std::string& s) {
  std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPCWrapperCall::ping" << reset << std::endl;

  grpc::ClientContext context;
  consensusInterface::Request request;
  consensusInterface::Response response;

  request.set_value(s);

  grpc::Status status = this->stub->ping(&context, request, &response);

  return (status.ok()) ? "OK" : "RPC failed !";
}
