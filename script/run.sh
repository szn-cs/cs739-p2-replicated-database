#!/bin/bash
# all command must be run on `Cloudlab.us` with `sudo su -`
# on an isntance of UBUNTU 20.04 with Temporary Filesystem Size of 40 GB mounted to /root
# copy over binaries from ./target/release

test() {
  # terminal 1
  {
    ./target/app --mode node
  }
  # terminal 2
  {
    ./target/app --mode user
  }
}

test_heartbeat() {
  source ./script/setenv.sh
  # SERVER_ADDRESS=c220g5-110912.wisc.cloudlab.us:50051

  ./target/app -g --port_consensus 8000 &
  ./target/app -g --port_consensus 8001 &
  ./target/app -g --port_consensus 8002 --port_database 8091 &
  ./target/app -g --port_consensus 8003 &
  ./target/app -g --port_consensus 8004 --flag.leader &

  # ./server $SERVER -serverAddress=$SERVER_ADDRESS >/dev/null 2>&1 &

  # Stop all background jobs
  # kill $(jobs -p)

  # Bring background to foreground
  # jobs
  # fg
  # bg
  # fg %1
}

test_rpc() {
  ./target/app -g --port_consensus 8000 --port_database 9000 &
  ./target/app -g --port_consensus 8001 --port_database 9001 &
  ./target/app -g --port_consensus 8002 --port_database 9002 &
  ./target/app -g --port_consensus 8003 --port_database 9003 &
  ./target/app -g --port_consensus 8004 --port_database 9004 --flag.leader &

  ./target/app --mode user --command set --key k1 --value v1 --target 0.0.0.0:8002
  ./target/app --mode user --command set --key k2 --value v2 --target 0.0.0.0:8004
}
