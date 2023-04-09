#include "./declaration.h"

namespace rpc {

/*ServerContext* context*/

  Status ConsensusRPC::propose(ServerContext* context, const consensus_interface::Request* request, consensus_interface::Response* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPC::propose" << reset << std::endl;

    // app::Consensus consensus = *app::Consensus::instance;

    if (context->IsCancelled()) {
      return Status(grpc::StatusCode::CANCELLED, "Deadline exceeded or Client cancelled, abandoning.");
    }

    // map<string, map<int, database_interface::LogEntry>> pax_log = consensus.Get_Log();

    string key = request->key();
    int round = request->round();
    string value = request->value();
    int p_server = request->pserver_id();

    if(app::Cluster::config->flag.debug){
      std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << 
        cyan << "Proposal is for key " << key << reset << std::endl;
    }
    // consensus.Set_Log(key, round, p_server);

    // // Check if the current proposal round is greater than the previously seen round for the given key
    // int prevRound = pax_log[key].size() - 1;
    // if (round <= prevRound) {
    //   response->set_status(Status_Types::FAILED);
    //   return Status::OK;
    // }

    // // requests from clients reach the propose stage
    // // multi paxos -> only leader proposes

    // // see if there exists a value for the given key with a proposal number greater than the current
    // pair<string, int> already_seen_key = consensus.Find_Max_Proposal(key, round);

    // if (already_seen_key.first != "") {  // the value is not nil ? then propose the same value
    //   round = already_seen_key.second + 1;
    //   value = already_seen_key.first;
    // }

    response->set_value(value);
    response->set_op(consensus_interface::SET_LEADER); // TODO: Hardcoded
    response->set_aserver_id(p_server);

    return Status::OK;
  }

  Status ConsensusRPC::accept(ServerContext* context, const consensus_interface::Request* request, consensus_interface::Response* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPC::accept" << reset << std::endl;

    if (context->IsCancelled()) {
      return Status(grpc::StatusCode::CANCELLED, "Deadline exceeded or Client cancelled, abandoning.");
    }

    // app::Consensus consensus = *app::Consensus::instance;

    // Extract the key and proposal no. from the request
    string key = request->key();
    int round = request->round();

    // Extract the proposal server ID, operation, and value from the request
    int pserver_id = request->pserver_id();
    int aserver_id = request->aserver_id();
    consensus_interface::Operation op = request->op();
    string value = request->value();

    if(app::Cluster::config->flag.debug){
      std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << 
        cyan << "Acceptance is for key " << key << " and value " << value << reset << std::endl;
    }

    // Lock the mutex for pax_log to prevent concurrent modifications
    //pthread_mutex_lock(&log_mutex);

    // map<string, map<int, database_interface::LogEntry>> pax_log = consensus.Get_Log();

    // // Check if a value has already been accepted for this key and round
    // if (pax_log.find(key) != pax_log.end()) {  // check if key exists in map
    //   auto it = pax_log[key].find(round);      // check if round exists for the key
    //   if (it != pax_log[key].end() && it->second.a_server_id() != -1) {
    //     // log of the third type exists for the key and round
    //     // it->second gives you the log entry for the key and round
    //     const database_interface::LogEntry entry = pax_log[key][round];
    //     if (entry.accepted_value() == value) {
    //       // If the acceptance information matches, return success
    //       response->set_status(Status_Types::OK);
    //     } else {
    //       // If the acceptance information does not match, return failure
    //       response->set_status(Status_Types::FAILED);
    //     }
    //   }
    // } else {
    //   database_interface::Operation op = database_interface::Operation::SET;
    //   // If a value has not been accepted for this key and round, add log indicating that a new value has been accepted
    //   consensus.Set_Log(key, round, aserver_id, op, value);
    // }

    // Unlock the mutex for pax_log
    //pthread_mutex_unlock(&log_mutex);

    response->set_op(op);
    response->set_value(value);
    response->set_pserver_id(pserver_id);
    response->set_aserver_id(aserver_id);
    response->set_round(round);


    return Status::OK;
  }

  Status ConsensusRPC::success(ServerContext* context, const consensus_interface::Request* request, consensus_interface::Empty* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPC::success" << reset << std::endl;
    if (context->IsCancelled()) {
      return Status(grpc::StatusCode::CANCELLED, "Deadline exceeded or Client cancelled, abandoning.");
    }

    std::string key = request->key();
    std::string value = request->value();

    if(app::Cluster::config->flag.debug){
      std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << 
        cyan << "Successful consensus, key " << key << " is now set to value " << value << reset << std::endl;
    }

    if(key == "leader"){
      app::Cluster::leader = value;
    }else{
      app::Database::instance->Set_KV(key, value);
    }

    return Status::OK;
  }

  Status ConsensusRPC::ping(ServerContext* context, const consensus_interface::Empty* request, consensus_interface::Empty* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPC::ping" << reset << std::endl;
    if (context->IsCancelled()) {
      return Status(grpc::StatusCode::CANCELLED, "Deadline exceeded or Client cancelled, abandoning.");
    }

   return Status::OK;
  }

  Status ConsensusRPC::db_address_request(ServerContext* context, const consensus_interface::Empty* request, consensus_interface::DbAddressResponse* response){
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPC::db_address_request" << reset << std::endl;
    if (context->IsCancelled()) {
      return Status(grpc::StatusCode::CANCELLED, "Deadline exceeded or Client cancelled, abandoning.");
    }

    // TODO: We may need this, or changes to the .ini file, to have variable port numbers for db
    return Status::OK;
  }

  Status ConsensusRPC::get_leader(ServerContext* context, const consensus_interface::Empty* request, consensus_interface::GetLeaderResponse* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPC::get_leader" << reset << std::endl;

    if (context->IsCancelled()) {
      return Status(grpc::StatusCode::CANCELLED, "Deadline exceeded or Client cancelled, abandoning.");
    }

    app::Consensus consensus = *app::Consensus::instance;

    std::string leader = consensus.GetLeader();
    response->set_leader(leader);
    return Status::OK;
  }

  Status ConsensusRPC::elect_leader(ServerContext* context, const consensus_interface::ElectLeaderRequest* request, consensus_interface::Empty* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPC::elect_leader" << reset << std::endl;

    if(context->IsCancelled()){
      return Status(grpc::StatusCode::CANCELLED, "Request timed out");
    }

    consensus_interface::Request r;
    r.set_value(request->value());
    r.set_key(request->key());
    r.set_op(consensus_interface::SET_LEADER);


    std::pair<Status, Response> resp = app::Consensus::instance->AttemptConsensus(r);
    
    if(resp.first.ok()){
      app::Cluster::leader = resp.second.value();
      // TODO: Log entry
    }

    return resp.first;
  }

  Status DatabaseRPC::get(ServerContext* context, const database_interface::GetRequest* request, database_interface::GetResponse* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "DatabaseRPC::get" << reset << std::endl;

    if (context->IsCancelled()) {
      return Status(grpc::StatusCode::CANCELLED, "Deadline exceeded or Client cancelled, abandoning.");
    }

    // Check if we are leader by asking consensus thread
    std::string addr = app::Cluster::leader;
    if(addr.empty()){
      return Status(grpc::StatusCode::ABORTED, "No leader set");
    }

    

    if (addr == app::Cluster::config->getAddress<app::Service::Consensus>().toString()) {
      // We are leader, return local value
      // Pair to store <value, error_code>. Useful in determining whether a key simply has an empty value or if they key does not exist
      // in the database, allowing us to return an error in the latter case
      std::pair<string, int> resp = app::Database::instance->Get_KV(request->key());
      if(app::Cluster::config->flag.debug){
        std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << 
          cyan << "We are leader." << reset << std::endl;
      }

      if (resp.second != 0) {
        response->set_value("");
        response->set_error(1);
        if(app::Cluster::config->flag.debug){
          std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << 
            cyan << "There was no value tied to key " << request->key() << reset << std::endl;
        }
      } else {
        if(app::Cluster::config->flag.debug){
          std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << 
            cyan << "Returning value " << resp.first << reset << std::endl;
        }
        response->set_value(resp.first);
      }

    } else {
      // We are not leader, contact the db stub of the leader, corresponding to the result of asking the consensus thread for the leader's address

      if(app::Cluster::config->flag.debug){
        std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << 
          cyan << "Not leader, forwarding request to leader." << reset << std::endl;
      }
      
      grpc::ClientContext leader_context;
      auto deadline =
          std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
      leader_context.set_deadline(deadline);
      

      database_interface::GetRequest leader_request;
      database_interface::GetResponse leader_response;

      leader_request.set_key(request->key());

      Status leader_status = app::Cluster::memberList->at(app::Cluster::leader)->databaseEndpoint.stub->stub->get(&leader_context, leader_request, &leader_response);

      if(leader_status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED ||
          leader_status.error_code() == grpc::StatusCode::CANCELLED){
        // TODO: We must elect a new leader

      } else{
        if(app::Cluster::config->flag.debug){
          std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << 
            cyan << "Leader replied with value " << leader_response.value() << reset << std::endl;
        }
        response->set_value(leader_response.value());
        response->set_error(leader_response.error());
      }
    }

    return Status::OK;
  }

  Status DatabaseRPC::set(ServerContext* context, const database_interface::SetRequest* request, database_interface::Empty* response) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "DatabaseRPC::set" << reset << std::endl;

    if (context->IsCancelled()) {
      return Status(grpc::StatusCode::CANCELLED, "Deadline exceeded or Client cancelled, abandoning.");
    }

    std::string addr = app::Cluster::leader;
    if(addr.empty()){
      std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << red << "Error: No leader set"<< reset << std::endl;
      return Status(grpc::StatusCode::ABORTED, "No leader set");
    }

    if (addr == app::Cluster::config->getAddress<app::Service::Consensus>().toString()) {
      // We are leader, trigger paxos algorithm
      std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << red 
        << "We are leader, triggering paxos to set key " << request->key() << " to value " << request->value() << reset << std::endl;
      
      Request req;
      req.set_key(request->key());
      req.set_value(request->value());

      std::pair<Status, Response> resp = app::Consensus::instance->AttemptConsensus(req);

      if(resp.first.ok()){

        if(app::Cluster::config->flag.debug){
           std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << 
            cyan << "Reached consensus: Key: " << request->key() << " Value: " << resp.second.value() << reset << std::endl;
        }
        //app::Database::instance->Set_KV(request->key(), resp.second.value());
      }
      return resp.first;
    } else {
      // We are not the leader, forward request to leader
      if(app::Cluster::config->flag.debug){
        std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << 
          red << "Not leader, forwarding request to leader." << reset << std::endl;
      }
      

      grpc::ClientContext leader_context;
      auto deadline =
          std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
      leader_context.set_deadline(deadline);
      

      database_interface::SetRequest leader_request;
      database_interface::Empty leader_response;

      leader_request.set_key(request->key());
      leader_request.set_value(request->value());

      Status leader_status = app::Cluster::memberList->at(app::Cluster::leader)->databaseEndpoint.stub->stub->set(&leader_context, leader_request, &leader_response);

      if(leader_status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED ||
          leader_status.error_code() == grpc::StatusCode::CANCELLED){
        // TODO: We must elect a new leader

      }
    }

    return Status::OK;
  }
}  // namespace rpc

namespace rpc::call {
  /* Database RPC wrappers ------------------------------------------------------------- */

  std::string DatabaseRPCWrapperCall::get(const std::string& s, bool deadline) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "DatabaseRPCWrapperCall::get" << reset << std::endl;

    grpc::ClientContext context;
    if(deadline){
      auto deadline =
          std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
      context.set_deadline(deadline);
    }

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

  Status DatabaseRPCWrapperCall::set(const std::string& key, const std::string& value, bool deadline) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "DatabaseRPCWrapperCall::set" << reset << std::endl;

    grpc::ClientContext context;

    if(deadline){
      auto deadline =
          std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
      context.set_deadline(deadline);
    }
    database_interface::SetRequest request;
    database_interface::Empty response;

    request.set_key(key);
    request.set_value(value);


    grpc::Status status = this->stub->set(&context, request, &response);
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << status.error_code() << " " << status.error_message() << reset << std::endl;

    // if (status.error_code() == grpc::StatusCode::UNAVAILABLE ||
    //     status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED) {
    //   // Elect new leader
    //   // Resend request to new leader
    // }

    return status;
  }

  /* Consensus RPC wrappers ------------------------------------------------------------- */

  std::pair<Status, Response> ConsensusRPCWrapperCall::propose(const Request request) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPCWrapperCall::propose" << reset << std::endl;
    ClientContext context;

    auto deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    context.set_deadline(deadline);
    Response response;

    grpc::Status status = this->stub->propose(&context, request, &response);

    std::pair<Status, Response> res;
    res.first = status;
    res.second = response;
    return res;
  }

  std::pair<Status, Response> ConsensusRPCWrapperCall::accept(const Request request) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPCWrapperCall::accept" << reset << std::endl;

    ClientContext context;
    auto deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    context.set_deadline(deadline);
    Response response;

    grpc::Status status = this->stub->accept(&context, request, &response);

    std::pair<Status, Response> res;
    res.first = status;
    res.second = response;
    return res;
  }

  std::pair<Status, std::string> ConsensusRPCWrapperCall::db_address_request() {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPCWrapperCall::db_address_request" << reset << std::endl;
    
    ClientContext context;
    auto deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    context.set_deadline(deadline);

    consensus_interface::Empty request;
    consensus_interface::DbAddressResponse response;

    grpc::Status status = this->stub->db_address_request(&context, request, &response);

    std::pair<Status, std::string> a;
    a.first = status;
    a.second = response.addr();

    return a;
  }

  Status ConsensusRPCWrapperCall::success(const Request request) {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPCWrapperCall::success" << reset << std::endl;

    grpc::ClientContext context;
    auto deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    context.set_deadline(deadline);

    consensus_interface::Empty response;

    grpc::Status status = this->stub->success(&context, request, &response);

    return status;
  }

  Status ConsensusRPCWrapperCall::ping() {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPCWrapperCall::ping" << reset << std::endl;

    grpc::ClientContext context;
    auto deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(500);
    context.set_deadline(deadline);

    consensus_interface::Empty request;
    consensus_interface::Empty response;

    grpc::Status status = this->stub->ping(&context, request, &response);

    return status;
  }

  std::pair<Status, std::string> ConsensusRPCWrapperCall::get_leader() {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPCWrapperCall::get_leader" << reset << std::endl;

    grpc::ClientContext context;
    auto deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(app::Cluster::config->timeout);
    context.set_deadline(deadline);

    consensus_interface::Empty request;
    consensus_interface::GetLeaderResponse response;

    grpc::Status status = this->stub->get_leader(&context, request, &response);

    std::pair<Status, std::string> res;
    res.first = status;
    res.second = response.leader();
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPCWrapperCall::get_leader returned " << response.leader() << reset << std::endl;
    return res;
  }

  Status ConsensusRPCWrapperCall::trigger_election() {
    std::cout << termcolor::grey << utility::getClockTime() << termcolor::reset << yellow << "ConsensusRPCWrapperCall::trigger_election" << reset << std::endl;

    grpc::ClientContext context;
    auto deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(5000);
    context.set_deadline(deadline);

    consensus_interface::ElectLeaderRequest request;
    consensus_interface::Empty response;

    request.set_key("leader");
    request.set_value(app::Cluster::config->getAddress<app::Service::Consensus>().toString());

    grpc::Status status = this->stub->elect_leader(&context, request, &response);

    return status;
  }

}  // namespace rpc::call
