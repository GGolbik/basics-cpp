@echo off

:: "Debug" or empty for release build
SET BUILD_TYPE=%1
SET BUILD_DIR=%~dp0
SET BUILD_DIR="%BUILD_DIR%build"
SET TOOLCHAIN_CMAKE="C:/Program Files/CMake/bin/cmake.exe"
::SET PATH=%PATH%C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin

IF NOT EXIST %BUILD_DIR% MKDIR %BUILD_DIR%

CD %BUILD_DIR%

@echo on

%TOOLCHAIN_CMAKE% -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..

%TOOLCHAIN_CMAKE% --build .