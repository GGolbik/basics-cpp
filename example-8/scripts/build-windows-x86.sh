#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

# "Debug" or empty for release build
BUILD_TYPE=$1
BUILD_DIR="${SCRIPT_DIR}/../build/windows-x86"
SRC_DIR=${SCRIPT_DIR}/../
OPENSSL_SCRIPT=${SCRIPT_DIR}/../libs/openssl-windows-x86.sh
TOOLCHAIN_CMAKE=$(which cmake)
TOOLCHAIN_FILE=${SCRIPT_DIR}/../cmake/windows-x86.cmake

if [[ ! -d ${BUILD_DIR} ]]; then
  mkdir -p ${BUILD_DIR}
fi

/bin/bash -c "set -o pipefail \
  && cd ${BUILD_DIR} \
  && bash ${OPENSSL_SCRIPT} \
  && ${TOOLCHAIN_CMAKE} -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ${SRC_DIR} \
  && ${TOOLCHAIN_CMAKE} --build ."

EXIT_CODE=$?
if [[ ${EXIT_CODE} -eq 0 ]]; then
  echo "Build was successful"
else
  echo "Build failed with ERROR: ${EXIT_CODE}"
  exit ${EXIT_CODE}
fi
