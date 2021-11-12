#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

# "Debug" or empty for release build
BUILD_TYPE=$1
BUILD_DIR="${SCRIPT_DIR}/build/default"
SRC_DIR=${SCRIPT_DIR}
TOOLCHAIN_CMAKE=$(which cmake)

if [[ ! -d ${BUILD_DIR} ]]; then
  mkdir -p ${BUILD_DIR}
fi

/bin/bash -c "set -o pipefail \
  && cd ${BUILD_DIR} \
  && ${TOOLCHAIN_CMAKE} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ${SRC_DIR} \
  && ${TOOLCHAIN_CMAKE} --build ."

EXIT_CODE=$?
if [[ ${EXIT_CODE} -eq 0 ]]; then
  echo "Build was successful"
else
  echo "Build failed with ERROR: ${EXIT_CODE}"
  exit ${EXIT_CODE}
fi
