# Distributed Replicated Database [cs739-p2-replicated-database]
- [Paxos implementation](./documentation/paxos-consensus.md)

# Setup: 

- execute `./script/provision-local.sh` to setup repo and install dependencies
- run script `build.sh` → binary files in `./target`
- check `run.sh` for running the program
  - to run funcitons in script files (without copy pasting): `$ (source ./script/<scriptname>.sh && <functioname>)`

# Resources & Research papers: 
- EPaxos 
  - <https://lamport.azurewebsites.net/pubs/paxos-simple.pdf>
  - <https://www.usenix.org/system/files/nsdi21-tollman.pdf>
  - [Piazza] Proving that Paxos implementation works correctly: <https://harmony.cs.cornell.edu/docs/textbook/paxos/>
  - [Piazza] List of tests for distributed system: <https://asatarin.github.io/testing-distributed-systems/>
  - A good summary of the protocol with some psuedo code: <https://martinfowler.com/articles/patterns-of-distributed-systems/paxos.html/>
  - [example C++] <https://github.com/jiahanzhu/PaxosKVStore>
  - [example Go] <https://github.com/kkdai/paxos> <https://github.com/xiang90/paxos> <https://github.com/kr/paxos> 
