#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

BUILD_DIR="${SCRIPT_DIR}/build"

# run tests
/bin/bash -c "set -o pipefail \
  && cd ${BUILD_DIR} \
  && make test"

# Check for test error
EXIT_CODE=$?
if [[ ${EXIT_CODE} -eq 0 ]]; then
  echo "Test was successful"
else
  echo "Test failed with ERROR: ${EXIT_CODE}"
  exit ${EXIT_CODE}
fi

# run code coverage
/bin/bash -c "set -o pipefail \
  && lcov --capture --directory ${BUILD_DIR} --output-file ${BUILD_DIR}/coverage.info \
  && genhtml ${BUILD_DIR}/coverage.info --output-directory ${BUILD_DIR}/coverage.html"

# Check for coverage error
EXIT_CODE=$?
if [[ ${EXIT_CODE} -eq 0 ]]; then
  echo "Coverage was successful"
else
  echo "Coverage failed with ERROR: ${EXIT_CODE}"
  exit ${EXIT_CODE}
fi
