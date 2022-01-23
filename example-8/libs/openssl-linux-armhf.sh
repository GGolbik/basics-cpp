#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

LIBS_DIR="${SCRIPT_DIR}"
BUILD_DIR="${SCRIPT_DIR}/../build/openssl-linux-armhf"
OPENSSL_DIR="${LIBS_DIR}/openssl"
TOOLCHAIN_MAKE=$(which make)
TARGET_NAME="linux-armv4"
CROSS_COMPILE_PREFIX="arm-linux-gnueabihf-"
DEST_DIR="${BUILD_DIR}/target"

# For OpenSSL 1.1.1d and 1.1.1e use:
#
# DEST_DIR="${BUILD_DIR}/target/usr/local"
#
# ./Configure ${TARGET_NAME} --prefix=${DEST_DIR} --openssldir=${DEST_DIR} --cross-compile-prefix=${CROSS_COMPILE_PREFIX} \
#
# and
#
# ${TOOLCHAIN_MAKE} install \
#
if [[ ! -d ${BUILD_DIR} ]]; then
  /bin/bash -c "set -o pipefail \
    && cd ${OPENSSL_DIR} \
    && ./Configure ${TARGET_NAME} --cross-compile-prefix=${CROSS_COMPILE_PREFIX} \
    && ${TOOLCHAIN_MAKE} \
    && ${TOOLCHAIN_MAKE} install DESTDIR=${DEST_DIR} \
    && ${TOOLCHAIN_MAKE} clean \
    && ${TOOLCHAIN_MAKE} distclean"
else
  echo "${BUILD_DIR} already exists."
fi
