#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

LIBS_DIR="${SCRIPT_DIR}/libs"
BUILD_DIR="${SCRIPT_DIR}/build/openssl"
OPENSSL_DIR="${LIBS_DIR}/openssl"
TOOLCHAIN_MAKE=$(which make)
#TARGET_NAME=linux-x86_64
TARGET_NAME=mingw64
#Note if you want 32-bit binaries you need to set the target to mingw and change to the i686 compiler. In other words: ./Configure --cross-compile-prefix=i686-w64-mingw32- mingw
CROSS_COMPILE_PREFIX=x86_64-w64-mingw32-

if [[ ! -d ${OPENSSL_DIR} ]]; then
  # The official OpenSSL Git Repository is located at git.openssl.org.
  # There is a GitHub mirror of the repository at github.com/openssl/openssl, which is updated automatically from the former on every commit.
  # https://www.openssl.org/source/
  #git clone git://git.openssl.org/openssl.git -b openssl-3.0.0 ${OPENSSL_DIR}
  git clone git://git.openssl.org/openssl.git -b OpenSSL_1_1_1d ${OPENSSL_DIR}
fi

# Prints available targets
${OPENSSL_DIR}/Configure LIST
