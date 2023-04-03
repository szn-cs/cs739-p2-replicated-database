#pragma once

#include "../include.h"
#include "../utility.h"
#include "./RPCWrapperCall.h"
#include "./entrypoint.h"
#include "Consensus.h"
#include "config.h"

extern Config config;

/**
 * Database RPC endpoint (which the server exports on a particular port)
*/
class DatabaseRPC : public databaseInterface::DatabaseService::Service {
 public:
  //  explicit DatabaseRPC() {
  //         pthread_mutex_init(&lock, NULL);
  //     }

  grpc::Status get(grpc::ServerContext*, const databaseInterface::Request*, databaseInterface::Response*) override;
  grpc::Status set(grpc::ServerContext*, const databaseInterface::Request*, databaseInterface::Response*) override;
  // // This just returns the current log and db snapshot to the Consensus thread, to be forwarded to a recovering replica
  // // This would only come from other servers
  // grpc::Status recovery(grpc::ServerContext*, const databaseInterface::RecoveryRequest*, databaseInterface::Recoveryresponse*) override;
};

/**
 * Represents the State Machine (where KV store that keeps the data in memory or persistent)
*/
class Database {
 private:
  // TODO: Lock for each entry to improve latency?
  map<string, string> kv_store;
  pthread_mutex_t data_mutex;

 public:
  // Get, set, and delete values in the kv store
  string Get_KV(const string& key);
  void Set_KV(const string& key, string& value);
  void Delete_KV(const string& key);
  map<string, string> Get_DB();
};
