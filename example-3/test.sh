#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

BUILD_DIR="${SCRIPT_DIR}/build"

# Workaround: Otherwise, test report contains only the last test case.
# https://github.com/google/googletest/issues/2506
# This is required when using default CTest integration because CMake enumerates the tests in the resulting executable and then generates individual CTest rules for running each test individually.
# Without the directory output, the tests will all write to the same filename and nothing works as you'd expect.
WORKAROUND=1

# run tests
if [[ ${WORKAROUND} -eq 0 ]]; then
  /bin/bash -c "set -o pipefail \
    && cd ${BUILD_DIR} \
    && make test"
else
  /bin/bash -c "set -o pipefail \
    && cd ${BUILD_DIR} \
    && test/project_cpp_test --gtest_output=xml:test/testreport.xml"
fi

# Check for test error
TEST_EXIT_CODE=$?
if [[ ${TEST_EXIT_CODE} -eq 0 ]]; then
  echo "Test was successful"
else
  echo "Test failed with ERROR: ${TEST_EXIT_CODE}"
fi

# create html test report
/bin/bash -c "set -o pipefail \
  && cd ${BUILD_DIR} \
  && xsltproc ${SCRIPT_DIR}/libs/gtest2html/gtest2html.xslt ${BUILD_DIR}/test/testreport.xml > ${BUILD_DIR}/test/testreport.html"

# Check for report error
EXIT_CODE=$?
if [[ ${EXIT_CODE} -eq 0 ]]; then
  echo "HTML test report has been created."
else
  echo "Failed to create HTML test report with ERROR: ${EXIT_CODE}"
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

exit $TEST_EXIT_CODE