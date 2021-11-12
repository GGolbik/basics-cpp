#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

IMAGE_BUILD_SCRIPT=${SCRIPT_DIR}/../docker-cpp-build/docker.sh
IMAGE_NAME=ggolbik/cpp-build:1.0
BUILD_DIR="${SCRIPT_DIR}/build"

/bin/bash ${IMAGE_BUILD_SCRIPT}
EXIT_CODE=$?
if [[ ${EXIT_CODE} -ne 0 ]]; then
  exit ${EXIT_CODE}
fi

CONTAINER_ID=$(docker run --interactive --tty --detach ${IMAGE_NAME})

mkdir -p ${BUILD_DIR}

docker cp ${SCRIPT_DIR}/. ${CONTAINER_ID}:/app

docker exec \
  --interactive \
  --tty \
  ${CONTAINER_ID} rm -r /app/build

docker exec \
  --interactive \
  --tty \
  ${CONTAINER_ID} bash /app/build.sh
docker exec \
  --interactive \
  --tty \
  ${CONTAINER_ID} find /app/scripts -name "*.sh" -exec {} \;

docker exec \
  --interactive \
  --tty \
  ${CONTAINER_ID} bash /app/install.sh

docker cp ${CONTAINER_ID}:/app/build/. ${BUILD_DIR}

docker stop ${CONTAINER_ID}

echo "Remove container:"
docker rm ${CONTAINER_ID}
