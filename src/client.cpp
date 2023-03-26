
#include "./client.h"
using namespace std;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan;

GRPC_Client::GRPC_Client(std::shared_ptr<Channel> channel) : stub_(ReplicatedDB::NewStub(channel)) {}

/** EXAMPLE: */
std::string GRPC_Client::get(const std::string& path) {
  // Data we are sending to the server.
  Request request;
  request.set_name(path);
  // Container for the data we expect from the server.
  Response response;
  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  // The actual RPC.
  Status status = stub_->get(&context, request, &response);

  // Act upon its status.
  if (status.ok()) {
    return response.message();
  } else {
    //std::cout << status.error_code() << ": " << status.error_message() << std::endl;
    return "RPC failed";
  }
}
