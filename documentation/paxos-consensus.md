# Multi-Paxos: distributes a log with enteries of single-decree instances. System picks a series of values. 
- Leader election process: depose to server with highest ID from heatbeats received. 
- Log distribution process

## config params: 
- concurrency limit
- 

## persisted: 
- **proposer**: 
  - maxRound
  - prepared: i.e. encountered noMoreAccepted msg
  - next index of entry to use for client request
- **Acceptor**: 
  - last log index
  - min proposal
  - log[{accepted proposal, accepted value}]
  - first unchosen index: earliest non ∞ entry

## API interface: 
- **Heartbeat RPC**: or use prepare with empty data. 
- **Prepare RPC**:  (log index, proposal #) → current enry accepted{proposal #, value}
- **Accept RPC**:  (log index, proporal{#, value}, first unchosen index in proposer) → minimum proposal, first unchosen index in acceptor
- **Success RPC**:  (index of corresponding acceptor, value) → new first unchosen index in acceptor

## Algorithm: 
- 