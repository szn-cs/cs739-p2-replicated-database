#include "Consensus.h"

#include <math.h>

using namespace std;

// string id;

// paxos_addresses_ : a list of network addresses of the Paxos servers , IP addresses and ports

extern Config config;

ConsensusRPCWrapperCall::ConsensusRPCWrapperCall(std::shared_ptr<Channel> channel)
    : stub(consensusInterface::ConsensusService::NewStub(channel)) {}

Status ConsensusRPC::propose(ServerContext* context, const consensusInterface::Request* request, consensusInterface::Response* response) {
  std::cout << yellow << "ConsensusRPC::propose" << reset << std::endl;

  if (context->IsCancelled()) {
    return Status(grpc::StatusCode::CANCELLED, "Deadline exceeded or Client cancelled, abandoning.");
  }

  // requests from clients reach the propose stage
  // multi paxos -> only leader proposes

  string key = request->key();
  string value = request->value();

  int round = 0;

  // see if there exists a value for the given key with a proposal number greater than the current

  pair<string, int> already_seen_value = Consensus::Find_Max_Proposal(key, round);

  //Set_Log(key, proposalNo);

  if (already_seen_value.first != "") {
    round = already_seen_value.second + 1;
    value = already_seen_value.first;
  }

  // retrieve maxRound
  //maxRound += 1;
  //string proposalNo = maxRound + serverId;

  //string path = "/tmp/server/maxRound.txt";
  //string round = readFromDisk(path);
  //int r = stoi(round) + 1;
  //string newRound = to_string(r);
  //writeToDisk(path, newRound);

  //string proposalNo = maxRound + id;

  //Set_Log(key, proposalNo);

  // proposer sends to all replicas for acceptance ,
  // if it gets majority acceptance
  // starts commiting

  return Status::OK;
}

Status ConsensusRPC::accept(ServerContext* context, const consensusInterface::Request* request, consensusInterface::Response* response) {
  std::cout << yellow << "ConsensusRPC::accept" << reset << std::endl;

  // Extract the key and proposal no. from the request
  string key = request->key();
  int round = request->round();

  // Extract the server ID, operation, and value from the request
  int server_id = request->pserver_id();
  consensusInterface::Operation op = request->op();
  string value = request->value();

  // Lock the mutex for pax_log to prevent concurrent modifications
  //pthread_mutex_lock(&log_mutex);

  map<string, map<int, databaseInterface::LogEntry>> pax_log = Consensus::Get_Log();

  // Check if a value has already been accepted for this key and round
  if (pax_log[key].count(round)) {
    // If a value has already been accepted, check if the acceptance information matches the request
    const databaseInterface::LogEntry entry = pax_log[key][round];
    if (entry.accepted_value() == value) {
      // If the acceptance information matches, return success
      response->set_status(Status_Types::OK);
    } else {
      // If the acceptance information does not match, return failure
      response->set_status(Status_Types::FAILED);
      return Status::CANCELLED;
    }
  } else {
    // If a value has not been accepted for this key and round, return failure
    response->set_status(Status_Types::FAILED);
    return Status::CANCELLED;
  }

  // Unlock the mutex for pax_log
  //pthread_mutex_unlock(&log_mutex);

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

// when Acceptor receives a proposal for particular key and round

void Consensus::Set_Log(const string& key, int round) {
  pthread_mutex_lock(&log_mutex);
  pax_log[key][round];
  pthread_mutex_unlock(&log_mutex);
}

// update proposal server

void Consensus::Set_Log(const string& key, int round, int p_server) {
  pthread_mutex_lock(&log_mutex);
  pax_log[key][round].set_p_server_id(p_server);
  pthread_mutex_unlock(&log_mutex);
}

// Update the acceptance info for the given key and round.

void Consensus::Set_Log(const string& key, int round, int a_server, databaseInterface::Operation op, string value) {
  pthread_mutex_lock(&log_mutex);
  pax_log[key][round].set_a_server_id(a_server);
  pax_log[key][round].set_op(op);
  pax_log[key][round].set_accepted_value(value);
  pthread_mutex_unlock(&log_mutex);
}

map<string, map<int, databaseInterface::LogEntry>> Consensus::Get_Log() {
}

// Find the highest proposal number seen for the given key
pair<string, int>
Consensus::Find_Max_Proposal(const string& key, int round) {
  map<string, map<int, databaseInterface::LogEntry>> pax_log = Consensus::Get_Log();

  //pthread_mutex_lock(&log_mutex);
  int max_proposal = round;
  for (const auto& entry : pax_log[key]) {
    if (entry.first > max_proposal) {
      max_proposal = entry.first;
    }
  }
  //pthread_mutex_unlock(&log_mutex);

  if (max_proposal == round)
    return make_pair("", 0);

  return make_pair(pax_log[key][max_proposal].accepted_value(), max_proposal);
}

string Consensus::readFromDisk(string path) {
  std::ifstream file(path);
  std::string value;
  std::getline(file, value);

  return value;
}

void Consensus::writeToDisk(string path, string value) {
  ofstream file2(path, std::ios::trunc);  // open the file for writing, truncate existing content
  file2 << value;                         // write the new content to the file
  file2.close();
}