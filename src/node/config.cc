#include "../include.h"

Node::Node(std::string consensusAddress) {
  // this->consensusEndpoint = Endpoint{consensusAddress};
}

Node::Node(std::string consensusAddress, std::string databaseAddress) {
  // this->consensusEndpoint = Endpoint<consensusInterface::ConsensusService::Stub>{consensusAddress};
  // this->databaseEndpoint = Endpoint<databaseInterface::DatabaseService::Stub>{databaseAddress};
}

Node::Node(Address consensus, Address database) /*: Node(consensus.getAddress(), database.getAddress()) */ {};

Cluster::Cluster(std::vector<std::string> addressList) {
  for (std::string& a : addressList) {
    Node n{a};
    this->memberList.push_back(n);
  }
}

Node node();
Cluster cluster();
