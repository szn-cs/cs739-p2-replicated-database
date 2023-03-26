#include "client.h"

static GRPC_Client* grpcClient;

int main(int argc, char* argv[]) {
  int r = 0;
  string serverAddress = "0.0.0.0:8080";  // target address & port to send grpc requests to.

  grpcClient = new GRPC_Client(grpc::CreateChannel(serverAddress, grpc::InsecureChannelCredentials()));

  string path = "filename";

  string message = grpcClient->get(path);

  std::cout << message << std::endl;

  return r;
}
