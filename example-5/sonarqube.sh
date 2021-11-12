#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

IMAGE_BUILD_SCRIPT=${SCRIPT_DIR}/../docker-sonarqube/docker.sh
IMAGE_NAME=ggolbik/sonarqube:1.0
SONAR_TOKEN=6d08afc3efdc568dcc7e7a8a3416f39fe990462b
SONAR_HOST_URL=http://localhost:9000
SRC_DIR=${SCRIPT_DIR}

/bin/bash ${IMAGE_BUILD_SCRIPT}
EXIT_CODE=$?
if [[ ${EXIT_CODE} -ne 0 ]]; then
  exit ${EXIT_CODE}
fi

docker run \
  --rm \
  -e SONAR_HOST_URL="${SONAR_HOST_URL}" \
  -e SONAR_LOGIN="${SONAR_TOKEN}" \
  -v "${SRC_DIR}:/usr/src" \
  --network host \
  sonarsource/sonar-scanner-cli

# Check for scan error
EXIT_CODE=$?
if [[ ${EXIT_CODE} -eq 0 ]]; then
  echo "Scan was successful"
else
  echo "Scan failed with ERROR: ${EXIT_CODE}"
  exit ${EXIT_CODE}
fi
