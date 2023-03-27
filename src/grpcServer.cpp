#include "./grpcServer.h"

Status gRPC_Server_DB::get(ServerContext* context, const Request* request, Response* response) {
  std::cout << yellow << "gRPC_Server_DB::get" << reset << std::endl;

  response->set_message("server recieved message: " + request->path());
  return Status::OK;
}

Status gRPC_Server_DB::set(ServerContext* context, const Request* request, Response* response) {
  std::cout << yellow << "gRPC_Server_DB::set" << reset << std::endl;

  return Status::OK;
}

Status gRPC_Server_Consensus::propose(ServerContext* context, const Request* request, Response* response) {
  std::cout << yellow << "gRPC_Server_Consensus::propose" << reset << std::endl;

  return Status::OK;
}

Status gRPC_Server_Consensus::accept(ServerContext* context, const Request* request, Response* response) {
  std::cout << yellow << "gRPC_Server_Consensus::accept" << reset << std::endl;

  return Status::OK;
}
