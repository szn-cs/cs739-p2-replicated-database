# Multi-Paxos: distributes a log with enteries of single-decree instances. System picks a series of values. 
- Leader election process: depose to server with highest ID from heatbeats received. If 2xT time passed, then act as leader. 
- Log distribution process
- DB atomic updates

## config params: 
- concurrency limit α
- Heartbeat every T milliseconds; T must be << round trip of message 

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
- **Heartbeat RPC** or use prepare with empty data. 
- **Prepare RPC**:  (log index, proposal #) → current enry accepted{proposal #, value}
- **Accept RPC**:  (log index, proporal{#, value}, first unchosen index in proposer) → minimum proposal, first unchosen index in acceptor
- **Success RPC**:  (index of corresponding acceptor, value) → new first unchosen index in acceptor

## Algorithm: 
### Prepare RPC
- Client uses unique id for each command (ensuring exactly once semantics, no duplicate exec.)
1. choose ealiest log entry empty (not chosen or accepted); 
  - proposal number = largest seen maxRound+1 concatenated with server ID; 
2. PrepareRPC(current log index, proposal # for entire log); 
3. Reject olrder proposal (block off entire log)
  - if highest proposal accepted minProposal < proposal #: then minProposal = proposal #; 
4. Return accepted for current entry & noMoreAccepted  (in case no proposal accepted beyond entry) 
5. if majority, replace/abandon own value with highest accepted proposal from reponses; 
  - if no majority, then no need for more prepares to server; 
### Accept RPC
1. broadcast accept RPC; 
2. proposal # >= minProposal: minProposal = proposal{#, value}; 
3. set all indecies < first unchosen index of proposer and have the same proposal # of proposer to ∞; 
4. return ... 
5. majority & response.minProposal <= Proposal: proposal value chosen (set to ∞); 
6. retry RPC until all acceptors respond (partial replication if crashes); no-op to qcuickly complete operation;
### Success RPC
- Run in background, allows acceptor to be caught up to speed; Repeat as necessary;
