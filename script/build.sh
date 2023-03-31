#!/bin/bash

# run using $` (source ./script/build.sh && build) `
build() {
  source ./script/setenv.sh

  # create make files &
  # build through `cmake`  or use `make -w -C ./target/config/`
  cmake -S . -B ./target/config && cmake --build ./target/config --parallel # --verbose
  ## move binaries from nested builds
  mkdir -p ./target/
  # copy binaries
  cp ./target/config/src/node ./target/
  cp ./target/config/src/user ./target/

  (source ./script/build.sh && test)
}

# TODO: move to CMAKE
test() {
  gcc -Wall ./test/test.c -o ./target/test
}

## clean
clean() {
  cmake --build ./target/config --target clean
}

install_package() {
  # ./dependency/vcpkg/vcpkg install ${package_name}
}
