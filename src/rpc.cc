#include "./declaration.h"

namespace rpc {

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

    std::shared_ptr<app::Consensus> instance = app::Consensus::instance;

    pair<string, int> already_seen_value = app::Consensus::instance->Find_Max_Proposal(key, round);

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

    map<string, map<int, databaseInterface::LogEntry>> pax_log = app::Consensus::instance->Get_Log();

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

  Status ConsensusRPC::ping(ServerContext* context, const consensusInterface::Request* request, consensusInterface::Response* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPC::ping" << reset << std::endl;

    cout << "recieved ping from: " << endl;

    return Status::OK;
  }

  Status ConsensusRPC::get_leader(ServerContext* context, const consensusInterface::Empty* request, consensusInterface::GetLeaderResponse* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPC::get_leader" << reset << std::endl;

    // string leader = Consensus::GetLeader();
    // // TODO: Error handling?
    // response->set_leader(leader);
    // return Status::OK;
  }

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
}  // namespace rpc

namespace rpc::call {
  /* Database RPC wrappers ------------------------------------------------------------- */

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

  std::string ConsensusRPCWrapperCall::ping(const std::string& s) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPCWrapperCall::ping" << reset << std::endl;

    grpc::ClientContext context;
    consensusInterface::Request request;
    consensusInterface::Response response;

    request.set_value(s);

    grpc::Status status = this->stub->ping(&context, request, &response);

    return (status.ok()) ? "OK" : "RPC failed !";
  }

}  // namespace rpc::call
