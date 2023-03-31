#include "entrypoint.h"

static DatabaseRPCWrapperCall* c;

/**
 * Handle configurations: 
 * 1. service database RPC endpoints/addresses
 * 2. coordination settings for testing
*/
int main(int argc, char* argv[]) {
  int r = 0;
  string db_address = "0.0.0.0:8081";  // target address & port to send grpc requests to.

  c = new DatabaseRPCWrapperCall(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

  string message = c->get("Hello message");

  std::cout << message << std::endl;

  return r;
}
