#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

LIBS_DIR="${SCRIPT_DIR}"
BUILD_DIR="${SCRIPT_DIR}/../build/openssl-windows-x86_64"
OPENSSL_DIR="${LIBS_DIR}/openssl"
TOOLCHAIN_MAKE=$(which make)
TARGET_NAME=mingw64
CROSS_COMPILE_PREFIX=x86_64-w64-mingw32-

if [[ ! -d ${BUILD_DIR} ]]; then
  /bin/bash -c "set -o pipefail \
    && cd ${OPENSSL_DIR} \
    && ./Configure ${TARGET_NAME} no-shared --prefix=${BUILD_DIR} --openssldir=${BUILD_DIR} --cross-compile-prefix=${CROSS_COMPILE_PREFIX} \
    && ${TOOLCHAIN_MAKE} \
    && ${TOOLCHAIN_MAKE} install \
    && ${TOOLCHAIN_MAKE} clean \
    && ${TOOLCHAIN_MAKE} distclean"
else
  echo "${BUILD_DIR} already exists."
fi
