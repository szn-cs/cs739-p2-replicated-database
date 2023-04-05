#include "./declaration.h"

namespace rpc {

  Status ConsensusRPC::propose(ServerContext* context, const consensus_interface::Request* request, consensus_interface::Response* response) {
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

  Status ConsensusRPC::accept(ServerContext* context, const consensus_interface::Request* request, consensus_interface::Response* response) {
    std::cout << yellow << "ConsensusRPC::accept" << reset << std::endl;

    // Extract the key and proposal no. from the request
    string key = request->key();
    int round = request->round();

    // Extract the server ID, operation, and value from the request
    int server_id = request->pserver_id();
    consensus_interface::Operation op = request->op();
    string value = request->value();

    // Lock the mutex for pax_log to prevent concurrent modifications
    //pthread_mutex_lock(&log_mutex);

    map<string, map<int, database_interface::LogEntry>> pax_log = app::Consensus::instance->Get_Log();

    // Check if a value has already been accepted for this key and round
    if (pax_log[key].count(round)) {
      // If a value has already been accepted, check if the acceptance information matches the request
      const database_interface::LogEntry entry = pax_log[key][round];
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

  Status ConsensusRPC::success(ServerContext* context, const consensus_interface::Request* request, consensus_interface::Response* response) {
    std::cout << yellow << "ConsensusRPC::accept" << reset << std::endl;

    return Status::OK;
  }

  Status ConsensusRPC::ping(ServerContext* context, const consensus_interface::Request* request, consensus_interface::Response* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPC::ping" << reset << std::endl;

    cout << "recieved ping from: " << endl;

    return Status::OK;
  }

  Status ConsensusRPC::get_leader(ServerContext* context, const consensus_interface::Empty* request, consensus_interface::GetLeaderResponse* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPC::get_leader" << reset << std::endl;

    // string leader = Consensus::GetLeader();
    // // TODO: Error handling?
    // response->set_leader(leader);
    // return Status::OK;
  }

  // NOTE: I hardcoded the address to work on my local easier. 0.0.0.0 also probably would have worked

  Status DatabaseRPC::get(ServerContext* context, const database_interface::GetRequest* request, database_interface::GetResponse* response) {
    std::cout << yellow << "DatabaseRPC::get" << reset << std::endl;

    // Check if we are leader by asking consensus thread
    rpc::call::ConsensusRPCWrapperCall* c = new rpc::call::ConsensusRPCWrapperCall(grpc::CreateChannel(app::Cluster::config->getLocalIP() + ":8000", grpc::InsecureChannelCredentials()));
    string addr;

    // TODO: this causes infinite loop when called
    // addr = c->get_leader();

    // Pair to store <value, error_code>. Useful in determining whether a key simply has an empty value or if they key does not exist
    // in the database, allowing us to return an error in the latter case
    std::pair<std::string, int>
        resp;
    resp.first = "";
    resp.second = 0;

    if (addr == app::Cluster::config->getLocalIP()) {
      // We are leader, return local value
      resp = app::Database::instance->Get_KV(request->key());
    } else {
      // We are not leader, contact the db stub of the leader, corresponding to the result of asking the consensus thread for the leader's address

      // Psuedo-ish code for what else needs to happen

      // DatabaseRPCWrapperCall* leader_c = cluster[message].second; // The database stub
      // value = leader_c.get(key);
      // Database::Set_KV(key, value);

      /*
        Potential issues:
          What happens if the leader is down? Ideally the getleader() call on line 16 would send a heartbeat to the leader, on the conseneus thread
          side, like after it receives the getleader() rpc, it looks at its locally cached leader address and sends a ping to it. If no response,
          it triggers an election, eventually returning the address of the leader to the database thread in the rpc response to getleader()
      */
    }

    if (resp.second == 1) {
      response->set_value("");
      response->set_error("DoesNotExist");
    } else {
      response->set_value(resp.first);
    }

    return Status::OK;
  }

  Status DatabaseRPC::set(ServerContext* context, const database_interface::SetRequest* request, database_interface::Empty* response) {
    std::cout << yellow << "DatabaseRPC::set" << reset << std::endl;

    // Dummy logic for now, gotta actually trigger a proposal
    app::Database::instance->Set_KV(request->key(), request->value());

    /*
      We implement this simply by sending a proposal rpc to the leader's consensus thread. The leader will handle reaching consensus, and will
      eventually return the agreed upon value.
      Only issue is if the leader is down. We need to implement timeouts, as shown in psuedo code in the RPCWrapperCall file in the database
      rpc calls code.
    */

    return Status::OK;
  }
}  // namespace rpc

namespace rpc::call {
  /* Database RPC wrappers ------------------------------------------------------------- */

  std::string DatabaseRPCWrapperCall::get(const std::string& s) {
    std::cout << yellow << "DatabaseRPCWrapperCall::get" << reset << std::endl;

    grpc::ClientContext context;
    // auto deadline =
    //     std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    // context.set_deadline(deadline);
    database_interface::GetRequest request;
    database_interface::GetResponse response;

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
    database_interface::SetRequest request;
    database_interface::Empty response;

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
    consensus_interface::Request request;
    consensus_interface::Response response;

    request.set_value(s);

    grpc::Status status = this->stub->ping(&context, request, &response);

    return (status.ok()) ? "OK" : "RPC failed !";
  }

  std::string ConsensusRPCWrapperCall::get_leader() {
    std::cout << yellow << "GetLeader::success" << reset << std::endl;
    grpc::ClientContext context;
    consensus_interface::Empty request;
    consensus_interface::GetLeaderResponse response;

    grpc::Status status = this->stub->get_leader(&context, request, &response);

    return (status.ok()) ? response.leader() : "RPC failed !";
    return "default";
  }

}  // namespace rpc::call
