# Use Debian 10 (Buster) as build container - https://hub.docker.com/_/debian
FROM debian:buster AS env

# Install the required tools
#
# The "build-essential" package is a reference for all the packages needed to compile a Debian package, e.g. g++, gcc and make.
# 
# "CMake" is an open-source, cross-platform family of tools designed to build, test and package software. CMake is used to control the software compilation process using simple platform and compiler independent configuration files, and generate native makefiles and workspaces that can be used in the compiler environment of your choice. 
# 
# "LCOV" is a graphical front-end for GCC's coverage testing tool gcov.
#
# "Git" is a distributed version-control system for tracking changes in source code during software development. It is used to access source code of dependencies.
#
# "Cppcheck" is an analysis tool for C/C++ code.
#
# "crossbuild-essential-<arch>"" and "mingw-w64" allows to build for a different architecture.
#
# "openssl" and "libssl-dev" provide SSL/TLS usage.
RUN /bin/bash -c set -o pipefail \
  && apt-get update \
  && apt-get install -y \
  build-essential \
  cmake \
  lcov \
  git \
  cppcheck \
  crossbuild-essential-amd64 \
  crossbuild-essential-armel \
  crossbuild-essential-armhf \
  crossbuild-essential-arm64 \
  mingw-w64 \
  openssl \
  libssl-dev

# Define arguments used in this dockerfile
ARG LABEL_CREATED

# Set labels - OCI Image Format https://www.opencontainers.org/
LABEL \
  # version of the packaged software
  org.opencontainers.image.version="1.0.0" \
  # date and time on which the image was built (string, date-time as defined by RFC 3339).
  org.opencontainers.image.created=${LABEL_CREATED} \
  # Name of the distributing entity, organization or individual.
  org.opencontainers.image.vendor="GGolbik" \
  # Text description of the image.
  org.opencontainers.image.description="C++ build container."

