#include "./server.h"

namespace fs = std::filesystem;
using namespace std;
using namespace grpcInterface;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::Status;  // https://grpc.github.io/grpc/core/md_doc_statuscodes.html
using grpcInterface::ReplicatedDB;

static std::string serverDirectory;
static std::unordered_map<std::string, int> serverclock;

Status GRPC_Server::get(ServerContext* context, const Path* request, HelloReply* response) {
  std::string prefix("Recieved path: ");
  response->set_message(prefix + request->path());
  return Status::OK;
}

void RunServer(std::string address) {
  GRPC_Server service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << termcolor::blue << "⚡ Server listening on " << address << termcolor::reset << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  struct stat info;

  // set defaults
  const std::string address("0.0.0.0:8080");
  serverDirectory = Utility::concatenatePath(fs::current_path().generic_string(), "tmp/server");

  // set configs from arguments
  if (argc == 2)
    serverDirectory = argv[1];

  // create directory if doesn't exist
  serverDirectory = fs::absolute(serverDirectory);
  fs::create_directories(serverDirectory);
  std::cout << blue << "serverDirectory: " << serverDirectory << reset << std::endl;

  RunServer(address);

  return 0;
}