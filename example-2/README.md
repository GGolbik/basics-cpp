# Basics C++ - Example 2

# Table of Contents

* [lcov](#lcov)
* [CMake Modifications](#cmake-modifications)
* [Build Project](#build-project)
* [Install Project](#install-project)
* [Test Project](#test-project)
* [Build and Install with Docker](#build-and-install-with-docker)

# lcov

`lcov` is a graphical front-end for GCC's coverage testing tool `gcov`.
It collects gcov data for multiple source files and creates HTML pages containing the source code annotated with coverage information.
It also adds overview pages for easy navigation within the file structure.

Install `lcov` (includes the `genhtml` tool to generate HTML output):
~~~
sudo apt-get install lcov
~~~

# CMake Modifications

You must call `enable_testing()` to be able to run tests
~~~
enable_testing()
~~~

Add test directory
~~~
add_subdirectory(test)
~~~

Create test project
~~~
# binary
add_executable(${PROJECT_TARGET_TEST} ${PROJECT_HEADERS} ${PROJECT_SOURCES} )
~~~

~~~
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
~~~

link the library of the application to test
~~~
target_link_libraries(${PROJECT_TARGET_TEST} PUBLIC ${PROJECT_TARGET_LIBRARY_TEST})
~~~

Set flags for coverage (`--coverage` command line argument is a shortcut for `-fprofile-arcs` and `-ftest-coverage`)
~~~
set_target_properties(${PROJECT_TARGET_TEST} PROPERTIES LINK_FLAGS  "-fprofile-arcs -ftest-coverage")

set_target_properties(${PROJECT_TARGET_TEST} PROPERTIES COMPILE_FLAGS  "-fprofile-arcs -ftest-coverage")
~~~

Add test
~~~
add_test(NAME ${PROJECT_TARGET_TEST} COMMAND ${PROJECT_TARGET_TEST})
~~~

To run tests
~~~
make test
~~~

Use lcov to collect coverage data:
~~~
lcov --capture --directory ./test/CMakeFiles/coveragetest.dir --output-file ./test/coverage.info
~~~

Use `genhtml` to create HTML pages.
~~~
genhtml ./test/coverage.info --output-directory ./test/coverage.html
~~~
*Note: Keep in mind that virtual methods must be implemented to reach 100%.*

# Build Project

Execute the `build.bat` script on Windows or `build.sh` script on Linux.

# Install Project

Execute the `install.sh` script on Linux. There is no support for Windows yet.

# Test Project

Execute the `test.sh` script on Linux. There is no support for Windows yet.

The code coverage output can be found in the build directory `build/coverage.html/index.html`.

# Build and Install with Docker

Execute the `docker.sh` script on Linux. There is no support for Windows yet.
