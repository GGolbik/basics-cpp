#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

IMAGE_NAME=ggolbik/cpp-build
IMAGE_TAG=1.0
BUILD_DIR="${SCRIPT_DIR}/build"
DOCKERFILE=${SCRIPT_DIR}/Dockerfile

# Create docker build container
if [[ "$(docker images -q ${IMAGE_NAME}:${IMAGE_TAG} 2>/dev/null)" == "" ]]; then
  docker build \
    --tag ${IMAGE_NAME} \
    --tag ${IMAGE_NAME}:${IMAGE_TAG} \
    --build-arg LABEL_CREATED=$(date -u +'%Y-%m-%dT%H:%M:%SZ') \
    --file ${DOCKERFILE} \
    ${SCRIPT_DIR}

  # Check for install error
  EXIT_CODE=$?
  if [[ ${EXIT_CODE} -eq 0 ]]; then
    echo "Docker build was successful"
  else
    echo "Docker build failed with ERROR: ${EXIT_CODE}"
    exit ${EXIT_CODE}
  fi
else
  echo "Docker image ${IMAGE_NAME}:${IMAGE_TAG} already exists."
fi
