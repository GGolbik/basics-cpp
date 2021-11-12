# Basics C++ - Example 7

# Table of Contents

* [Build Project](#build-project)
* [Install Project](#install-project)
* [Build and Install with Docker](#build-and-install-with-docker)
* [Cross Toolchain](#cross-toolchain)

# Build Project

Execute the `build.bat` script on Windows or `build.sh` script on Linux.

# Install Project

Execute the `build.sh` script on Linux. There is no support for Windows yet.

# Build and Install with Docker

Execute the `docker.sh` script on Linux. There is no support for Windows yet.

# Cross Toolchain

In the `cmake` directory are CMake toolchain files for each target:
- `build-linux-amd64.sh`
- `build-linux-arm64.sh`
- `build-linux-armel.sh`
- `build-linux-armhf.sh`
- `build-windows-x86_64.sh`
- `build-windows-x86.sh`

In the `script` directory are build scripts for each target which will use the toolchain files.

The docker script executes all scripts in the directory.

`-DCMAKE_SYSTEM_NAME`: name of the platform, for which we are building (target platform), `Linux`, `Windows` or `Darwin` for macOS. By default, this value is the same as `CMAKE_HOST_SYSTEM_NAME`, which means that we are building for local platform (no cross-compilation).
If your target is an embedded system without OS set `CMAKE_SYSTEM_NAME` to `Generic`.