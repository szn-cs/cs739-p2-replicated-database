#include "client.h"

static GRPC_Client* grpcClient;

int main(int argc, char* argv[]) {
  int r = 0;
  string replicated_db_address = "0.0.0.0:8081";  // target address & port to send grpc requests to.

  grpcClient = new GRPC_Client(grpc::CreateChannel(replicated_db_address, grpc::InsecureChannelCredentials()));

  string message = grpcClient->get("filename");

  std::cout << message << std::endl;

  return r;
}
