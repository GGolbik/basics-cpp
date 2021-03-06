#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

LIBS_DIR="${SCRIPT_DIR}/libs"
GTEST_DIR="${LIBS_DIR}/googletest"
GTEST2HTML="${LIBS_DIR}/gtest2html"

if [[ ! -d ${GTEST_DIR} ]]; then
  git clone https://github.com/google/googletest.git -b release-1.11.0 ${GTEST_DIR}
fi

# Optional html generator
#if [[ ! -d ${GTEST2HTML} ]]; then
#  git clone https://gitlab.uni-koblenz.de/agrt/gtest2html.git ${GTEST2HTML}
#fi
