#include "./grpcServer.h"

// struct Command {
//   key : " ";  // leader key or custom ...
//   value : " ";
// }

// log DS
// Vector<Command> log;

Status gRPC_Server_DB::get(ServerContext* context, const Request* request, Response* response) {
  std::cout << yellow << "gRPC_Server_DB::get" << reset << std::endl;

  response->set_message("server recieved message: " + request->path());
  return Status::OK;
}

Status gRPC_Server_DB::set(ServerContext* context, const Request* request, Response* response) {
  std::cout << yellow << "gRPC_Server_DB::set" << reset << std::endl;

  // loop over the nodes the try to get a majority consensus
  // grpc req through

  return Status::OK;
}

Status gRPC_Server_Consensus::propose(ServerContext* context, const Request* request, Response* response) {
  std::cout << yellow << "gRPC_Server_Consensus::propose" << reset << std::endl;

  //

  return Status::OK;
}

Status gRPC_Server_Consensus::accept(ServerContext* context, const Request* request, Response* response) {
  std::cout << yellow << "gRPC_Server_Consensus::accept" << reset << std::endl;

  return Status::OK;
}

Status gRPC_Server_Consensus::success(ServerContext* context, const Request* request, Response* response) {
  std::cout << yellow << "gRPC_Server_Consensus::accept" << reset << std::endl;

  return Status::OK;
}

Status gRPC_Server_Consensus::heartbeat(ServerContext* context, const Request* request, Response* response) {
  std::cout << yellow << "gRPC_Server_Consensus::accept" << reset << std::endl;

  return Status::OK;
}
