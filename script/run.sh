#!/bin/bash
# all command must be run on `Cloudlab.us` with `sudo su -`
# on an isntance of UBUNTU 20.04 with Temporary Filesystem Size of 40 GB mounted to /root
# copy over binaries from ./target/release

server() {
  # SERVER_ADDRESS=c220g5-110912.wisc.cloudlab.us:50051
  SERVER_ADDRESS=0.0.0.0:8080
  source ./script/setenv.sh

  ./server $SERVER -serverAddress=$SERVER_ADDRESS
}

server_background() {
  # SERVER_ADDRESS=c220g5-110912.wisc.cloudlab.us:50051
  SERVER_ADDRESS=0.0.0.0:8080
  source ./script/setenv.sh

  ./server $SERVER -serverAddress=$SERVER_ADDRESS >/dev/null 2>&1 &
}
