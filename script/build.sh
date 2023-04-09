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
  cp ./target/config/app ./target/
  # cp ./target/config/user ./target/
  cp ./config/*.ini ./target/

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
  VCPKG=./dependency/vcpkg
  # ./${VCPKG}/vcpkg install ${package_name}

  ################################################

  # versioning -----------------------------------
  # https://learn.microsoft.com/en-us/vcpkg/users/examples/modify-baseline-to-pin-old-boost
  # resolve versioning https://www.appsloveworld.com/cplus/100/197/cmake-new-boost-version-may-have-incorrect-or-missing-dependencies-and-imported
  # get builtin-baseline for vcpkg.json
  # https://learn.microsoft.com/en-us/vcpkg/users/examples/versioning.getting-started#builtin-baseline
  # (cd ${VCPKG} && git rev-parse HEAD)
}
