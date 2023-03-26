#pragma once

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <termcolor/termcolor.hpp>
#include <vector>

#include "Utility.cpp"
#include "grpcInterface.grpc.pb.h"

#define CHUNK_SIZE 1572864

namespace fs = std::filesystem;
using namespace std;
using namespace grpc;
using namespace grpcInterface;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::Status;  // https://grpc.github.io/grpc/core/md_doc_statuscodes.html
using grpcInterface::ReplicatedDB;

// Logic and data behind the server's behavior.
class GRPC_Server final : public ReplicatedDB::Service {
  // pthread_mutex_t lock;
 public:
  //  explicit GRPC_Server() {
  //         pthread_mutex_init(&lock, NULL);
  //     }

  Status get(ServerContext*, const Request*, Response*) override;
};