#!/bin/bash
# all command must be run on `Cloudlab.us` with `sudo su -`
# on an isntance of UBUNTU 20.04 with Temporary Filesystem Size of 40 GB mounted to /root
# copy over binaries from ./target/release

test() {
  # terminal 1
  {
    ./target/node
  }
  # terminal 2
  {
    ./target/user
  }
}

test_heartbeat() {
  source ./script/setenv.sh
  # SERVER_ADDRESS=c220g5-110912.wisc.cloudlab.us:50051

  ./target/node --port_consensus 8080 &
  ./target/node --port_consensus 8081 &
  ./target/node --port_consensus 8082 &
  ./target/node --port_consensus 8083 &
  ./target/node --port_consensus 8084 --flag.leader &

  # ./server $SERVER -serverAddress=$SERVER_ADDRESS >/dev/null 2>&1 &
}
