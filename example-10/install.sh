#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

BUILD_DIR="${SCRIPT_DIR}/build"
DEST_DIR="${BUILD_DIR}/target"

# install project into target directory
/bin/bash -c "set -o pipefail \
  && cd ${BUILD_DIR} \
  && make install DESTDIR=${DEST_DIR}"

# Check for install error
EXIT_CODE=$?
if [[ ${EXIT_CODE} -eq 0 ]]; then
  echo "Install was successful"
else
  echo "Install failed with ERROR: ${EXIT_CODE}"
  exit ${EXIT_CODE}
fi