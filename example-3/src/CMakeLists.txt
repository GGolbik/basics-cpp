###########################################################
# files
# In this section we basically specify all the files and club then into sensible variable names like source, include, etc. It is just to ease things out, but if you wish you can totally skip this section and use the file names directly instead of the variables.
###########################################################
message(STATUS "### Files ###")

message(STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")

# Find all source files
file(GLOB_RECURSE PROJECT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
message(STATUS "PROJECT_SOURCES=${PROJECT_SOURCES}")

# Find all header files
file(GLOB_RECURSE PROJECT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h" )
message(STATUS "PROJECT_HEADERS=${PROJECT_HEADERS}")





###########################################################
# target
# This is the part where we tell CMake the name of the output file, in our case we wish to name it as binary. Whatever files names follow after that are basically your source files same way as you do while compiling them manually.
###########################################################
message(STATUS "### Target ###")

# defines our binary
add_executable(${PROJECT_TARGET_BINARY} ${PROJECT_HEADERS} ${PROJECT_SOURCES} )
# set version number
set_target_properties(${PROJECT_TARGET_BINARY} PROPERTIES VERSION ${PROJECT_VERSION})
# set language
set_target_properties(${PROJECT_TARGET_BINARY} PROPERTIES LINKER_LANGUAGE CXX)

# defines our test library
add_library(${PROJECT_TARGET_LIBRARY_TEST} STATIC ${PROJECT_HEADERS} ${PROJECT_SOURCES})
# set version number
set_target_properties(${PROJECT_TARGET_LIBRARY_TEST} PROPERTIES VERSION ${PROJECT_VERSION})

# The COMMAND to run gcov is possible thanks to the -fprofile-arcs -ftest-coverage compile flags
set_target_properties(${PROJECT_TARGET_LIBRARY_TEST} PROPERTIES LINK_FLAGS  "-fprofile-arcs -ftest-coverage")
set_target_properties(${PROJECT_TARGET_LIBRARY_TEST} PROPERTIES COMPILE_FLAGS  "-fprofile-arcs -ftest-coverage")


###########################################################
# include
# This command is used to specify the path of the include directories that you want the compiler to look into while searching for header files while compiling your code. This will also include the header files from 3rd party libraries as we have done for Randomize and Logger.
###########################################################






###########################################################
# install
# The TARGETS form specifies rules for installing targets from a project. There are several kinds of target files that may be installed:
###########################################################
message(STATUS "### Install ###")

install(TARGETS ${PROJECT_TARGET_BINARY} RUNTIME DESTINATION bin)