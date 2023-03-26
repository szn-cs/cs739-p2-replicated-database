#!/bin/bash

# run using $` (source ./script/build.sh && build) `
build() {
  source ./script/setenv.sh

  # create make files &
  # build through `cmake`  or use `make -w -C ./target/config/`
  cmake -S . -B ./target/config && cmake --build ./target/config --parallel # --verbose
  ## move binaries from nested builds
  mkdir -p ./target/release
  # copy binaries
  cp ./target/config/server ./target/release/server

  (source ./script/build.sh && test)
}

# TODO: move to CMAKE
test() {
  gcc -Wall ./test/test.c -o ./target/test
}

## clean
# cmake --build ./target/config --target clean
