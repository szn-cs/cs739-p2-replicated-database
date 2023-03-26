#include "./grpcServer.h"

Status GRPC_Server::get(ServerContext* context, const Request* request, Response* response) {
  std::cout << yellow << "GRPC_Server::get" << reset << std::endl;

  response->set_message("server recieved message: " + request->path());
  return Status::OK;
}
