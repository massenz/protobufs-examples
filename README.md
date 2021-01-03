# Template C++ Project


Project   | C++ Protocol Buffers Examples
:---      | ---:
Author    | [M. Massenzio](https://bitbucket.org/marco)
Release   | 0.1.0
Updated   | 2021-01-02


# Overview

Example code to try out Protobuf's functionality.

# Install & Build

All the classes are documented using [Doxygen](http://www.doxygen.nl/), just run:

    $ doxygen

from the main project directory and all apidocs will be generated in `docs/apidocs/html`.

The scripts in this repository take advantage of [shared common utility functions](https://bitbucket.org/marco/common-utils): make `$COMMON_UTILS_DIR` point to it after cloning it:

```shell
pushd /some/path

git clone git@bitbucket.org:marco/common-utils.git
export COMMON_UTILS_DIR="$(pwd)/common-utils"

popd

ln -s ${COMMON_UTILS_DIR}/build.sh bin/build && \
    ln -s ${COMMON_UTILS_DIR}/test.sh bin/test
```

