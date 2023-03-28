#include "entrypoint.h"

static std::unordered_map<std::string, int> serverclock;
static std::string serverDirectory;

template <typename S>
void run_gRPC_server(std::string address) {
  S service;

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

auto runConsensusServer = [](std::string address) { run_gRPC_server<DatabaseRPC>(address); };
auto runDBServer = [](std::string address) { run_gRPC_server<ConsensusRPC>(address); };

int main(int argc, char** argv) {
  struct stat info;

  // set defaults
  const std::string address_consensus("0.0.0.0:8080");
  const std::string address_db("0.0.0.0:8081");

  serverDirectory = Utility::concatenatePath(fs::current_path().generic_string(), "tmp/server");

  // TODO: add list of address of quorum
  // TODO: debug flag: debugging outputs, DB configuration modes.

  // set configs from arguments
  if (argc == 2)
    serverDirectory = argv[1];

  // create directory if doesn't exist
  serverDirectory = fs::absolute(serverDirectory);
  fs::create_directories(serverDirectory);
  std::cout << blue << "serverDirectory: " << serverDirectory << reset << std::endl;

  std::thread consensus(runDBServer, address_consensus);
  std::thread db(runConsensusServer, address_db);

  db.join();
  consensus.join();

  return 0;
}