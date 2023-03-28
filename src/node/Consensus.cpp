#include "./Consensus.h"

ConsensusClientRPC::ConsensusClientRPC(std::shared_ptr<Channel> channel)
    : stub(consensusInterface::ConsensusService::NewStub(channel)) {}

Status ConsensusRPC::propose(ServerContext* context, const consensusInterface::Request* request, consensusInterface::Response* response) {
  std::cout << yellow << "ConsensusRPC::propose" << reset << std::endl;

  return Status::OK;
}

Status ConsensusRPC::accept(ServerContext* context, const consensusInterface::Request* request, consensusInterface::Response* response) {
  std::cout << yellow << "ConsensusRPC::accept" << reset << std::endl;

  return Status::OK;
}

Status ConsensusRPC::success(ServerContext* context, const consensusInterface::Request* request, consensusInterface::Response* response) {
  std::cout << yellow << "ConsensusRPC::accept" << reset << std::endl;

  return Status::OK;
}

Status ConsensusRPC::heartbeat(ServerContext* context, const consensusInterface::Request* request, consensusInterface::Response* response) {
  std::cout << yellow << "ConsensusRPC::accept" << reset << std::endl;

  return Status::OK;
}

std::string ConsensusClientRPC::propose(const std::string& s) {
  std::cout << yellow << "ConsensusClientRPC::propose" << reset << std::endl;

  return "default";
}

std::string ConsensusClientRPC::accept(const std::string& s) {
  std::cout << yellow << "ConsensusClientRPC::accept" << reset << std::endl;

  return "default";
}

std::string ConsensusClientRPC::success(const std::string& s) {
  std::cout << yellow << "ConsensusClientRPC::success" << reset << std::endl;

  return "default";
}

std::string ConsensusClientRPC::heartbeat(const std::string& s) {
  std::cout << yellow << "ConsensusClientRPC::heartbeat" << reset << std::endl;

  return "default";
}