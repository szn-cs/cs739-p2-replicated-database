#include "entrypoint.h"

static ClientRPC* c;

int main(int argc, char* argv[]) {
  int r = 0;
  string db_address = "0.0.0.0:8081";  // target address & port to send grpc requests to.

  c = new ClientRPC(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

  string message = c->get("filename");

  std::cout << message << std::endl;

  return r;
}
