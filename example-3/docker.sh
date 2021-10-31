#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

IMAGE_NAME=ggolbik/cpp-build
BUILD_DIR="${SCRIPT_DIR}/build"

# Create docker build container
if [[ "$(docker images -q ${IMAGE_NAME} 2> /dev/null)" == "" ]]; then
  docker build \
    --tag ${IMAGE_NAME} \
    --build-arg LABEL_CREATED=$(date -u +'%Y-%m-%dT%H:%M:%SZ') \
    ${SCRIPT_DIR}/../

  # Check for install error
  EXIT_CODE=$?
  if [[ ${EXIT_CODE} -eq 0 ]]; then
    echo "Docker build was successful"
  else
    echo "Docker build failed with ERROR: ${EXIT_CODE}"
    exit ${EXIT_CODE}
  fi
else
  echo "Docker image ${IMAGE_NAME} already exists."
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
  ${CONTAINER_ID} bash /app/libs.sh

docker exec \
  --interactive \
  --tty \
  ${CONTAINER_ID} bash /app/build.sh

docker exec \
  --interactive \
  --tty \
  ${CONTAINER_ID} bash /app/install.sh

docker exec \
  --interactive \
  --tty \
  ${CONTAINER_ID} bash /app/test.sh

docker cp ${CONTAINER_ID}:/app/build/. ${BUILD_DIR}

docker stop ${CONTAINER_ID}

docker rm ${CONTAINER_ID}
