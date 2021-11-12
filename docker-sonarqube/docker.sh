#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

IMAGE_NAME=ggolbik/sonarqube
IMAGE_TAG=1.0
BUILD_DIR="${SCRIPT_DIR}/build"
DOCKERFILE=${SCRIPT_DIR}/Dockerfile
PLUGIN_URL=https://github.com/SonarOpenCommunity/sonar-cxx/releases/download/cxx-2.0.5/sonar-cxx-plugin-2.0.5.2867.jar
PLUGIN_SONAR_CXX_FILENAME=sonar-cxx-plugin-2.0.5.2867.jar
CONTAINER_NAME=sonarqube

# Create docker build container
if [[ "$(docker images -q ${IMAGE_NAME}:${IMAGE_TAG} 2>/dev/null)" == "" ]]; then

  # create build dir
  mkdir -p ${BUILD_DIR}

  # download plugin
  wget -q -nc -O ${BUILD_DIR}/${PLUGIN_SONAR_CXX_FILENAME} ${PLUGIN_URL}

  # build container
  docker build \
    --tag ${IMAGE_NAME} \
    --tag ${IMAGE_NAME}:${IMAGE_TAG} \
    --build-arg LABEL_CREATED=$(date -u +'%Y-%m-%dT%H:%M:%SZ') \
    --build-arg PLUGIN_SONAR_CXX_FILENAME=build/${PLUGIN_SONAR_CXX_FILENAME} \
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

if [ ! "$(docker ps -a | grep ${CONTAINER_NAME})" ]; then
  # start container
  docker run \
    --detach \
    --stop-timeout 3600 \
    --name ${CONTAINER_NAME} \
    -e SONAR_ES_BOOTSTRAP_CHECKS_DISABLE=true \
    -p 9000:9000 \
    ${IMAGE_NAME}
else
  echo "Container ${CONTAINER_NAME} is already running."
fi

echo "Go to http://localhost:9000/ and login with user=admin and password=admin"
