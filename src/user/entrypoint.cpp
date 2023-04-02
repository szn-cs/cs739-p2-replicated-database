#pragma once
#include <iostream>

#include "../Utility.cpp"
#include "../node/RPCWrapperCall.h"

namespace fs = std::filesystem;
using namespace std;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan;

namespace user {

/**
 * Handle configurations: 
 * 1. service database RPC endpoints/addresses
 * 2. coordination settings for testing
*/
int main(int argc, char* argv[]) {
  int r = 0;
  string db_address = "0.0.0.0:8081";  // target address & port to send grpc requests to.

  DatabaseRPCWrapperCall* c = new DatabaseRPCWrapperCall(grpc::CreateChannel(db_address, grpc::InsecureChannelCredentials()));

  string message = c->get("Hello message");

  std::cout << message << std::endl;

  return r;
}

}  // namespace user
