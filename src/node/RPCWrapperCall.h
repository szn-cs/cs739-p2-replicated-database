#pragma once

#include "../include.h"
#include "../utility.h"

/** 
 * Database
 * Wraps RPC calls (acts as client) to the RPC endpoint (server)
*/
class DatabaseRPCWrapperCall {
 public:
  DatabaseRPCWrapperCall(std::shared_ptr<grpc::Channel> channel);

  /** database calls*/
  std::string get(const std::string&);
  std::string set(const std::string&);

 private:
  std::unique_ptr<databaseInterface::DatabaseService::Stub> stub;
};

/**
 * Consensus 
 * Wraps RPC calls (client) to the RPC endpoint (server)
*/
class ConsensusRPCWrapperCall {
 public:
  ConsensusRPCWrapperCall(std::shared_ptr<grpc::Channel> channel);

  /** consensus calls */
  std::string propose(const std::string&);
  std::string accept(const std::string&);
  std::string success(const std::string&);
  std::string ping(const std::string&);

 private:
  std::unique_ptr<consensusInterface::ConsensusService::Stub> stub;
};
