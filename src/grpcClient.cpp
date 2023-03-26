
#include "./grpcClient.h"

GRPC_Client::GRPC_Client(std::shared_ptr<Channel> channel) : stub_(ReplicatedDB::NewStub(channel)) {}

/** EXAMPLE: */
std::string GRPC_Client::get(const std::string& path) {
  std::cout << yellow << "GRPC_Client::get" << reset << std::endl;

  ClientContext context;
  Request request;
  Response response;

  request.set_path(path);

  Status status = stub_->get(&context, request, &response);

  return (status.ok()) ? response.message() : "RPC failure";
}
