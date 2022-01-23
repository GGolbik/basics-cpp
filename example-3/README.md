# Basics C++ - Example 3

# Table of Contents

* [lcov](#lcov)
* [CMake Modifications](#cmake-modifications)
* [Build Project](#build-project)
* [Install Project](#install-project)
* [Test Project](#test-project)
* [Build, Install and Test with Docker](#build-install-and-test-with-docker)

# lcov

`lcov` is a graphical front-end for GCC's coverage testing tool `gcov`.
It collects gcov data for multiple source files and creates HTML pages containing the source code annotated with coverage information.
It also adds overview pages for easy navigation within the file structure.

Install `lcov` (includes the `genhtml` tool to generate HTML output):
~~~
sudo apt-get install lcov
~~~

# GoogleTest

GoogleTest, Google's C++ test framework is available on [GitHub](https://github.com/google/googletest.git) as well as the [documentation](https://google.github.io/googletest/quickstart-cmake.html).

The dependencies can be downloaded by executing the `libs.sh` script on Linux. There is no support for Windows yet.

~~~
#include <gtest/gtest.h>

int main(int argc, char **argv)
{
  // The ::testing::InitGoogleTest() function parses the command line for googletest flags, and removes all recognized flags. This allows the user to control a test program’s behavior via various flags,
  ::testing::InitGoogleTest(&argc, argv);
  // RUN_ALL_TESTS() runs all tests
  return RUN_ALL_TESTS();
}
~~~

Demonstrate some basic assertions:
~~~
TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}
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

link the googletest library of the application to test
~~~
target_link_libraries(${PROJECT_TARGET_TEST} PUBLIC gtest_main)
~~~

Set flags for coverage (`--coverage` command line argument is a shortcut for `-fprofile-arcs` and `-ftest-coverage`)
~~~
set_target_properties(${PROJECT_TARGET_TEST} PROPERTIES LINK_FLAGS  "-fprofile-arcs -ftest-coverage")

set_target_properties(${PROJECT_TARGET_TEST} PROPERTIES COMPILE_FLAGS  "-fprofile-arcs -ftest-coverage")
~~~

Add test
~~~
include(GoogleTest)
gtest_discover_tests(${PROJECT_TARGET_TEST} EXTRA_ARGS "--gtest_output=xml:testreport.xml")
~~~

To run tests
~~~
make test
~~~

Use `lcov` to collect coverage data:
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

The test report can be found in the build directory `build/test/testreport.html`.

## gtest2html

Convert googletest xml output to html with [gtest2html](https://github.com/adarmalik/gtest2html)

~~~
sudo apt install xsltproc
~~~

~~~
xsltproc libs/gtest2html/gtest2html.xslt build/test/testreport.xml > build/test/testreport.html
~~~

You can use the alternative [gtest2html generator](https://gitlab.uni-koblenz.de/agrt/gtest2html).
~~~
python3 libs/gtest2html/gtest2html.py build/test/testreport.xml build/test/index.html
~~~

# Build, Install and Test with Docker

Execute the `docker.sh` script on Linux. There is no support for Windows yet.
