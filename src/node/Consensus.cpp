#include "./Consensus.h"

extern Config config;

ConsensusRPCWrapperCall::ConsensusRPCWrapperCall(std::shared_ptr<Channel> channel)
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

  return "default";
}

// Methods for adding to log at different points during paxos algorithm
void Consensus::Set_Log(const string& key, int round) {
  pthread_mutex_lock(&log_mutex);
  pax_log[key][round];
  pthread_mutex_unlock(&log_mutex);
}

void Consensus::Set_Log(const string& key, int round, int p_server) {
  pthread_mutex_lock(&log_mutex);
  pax_log[key][round].set_p_server_id(p_server);
  pthread_mutex_unlock(&log_mutex);
}

void Consensus::Set_Log(const string& key, int round, int a_server, databaseInterface::Operation op, string value) {
  pthread_mutex_lock(&log_mutex);
  pax_log[key][round].set_a_server_id(a_server);
  pax_log[key][round].set_op(op);
  pax_log[key][round].set_accepted_value(value);
  pthread_mutex_unlock(&log_mutex);
}
