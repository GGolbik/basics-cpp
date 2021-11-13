###########################################################
# files
# In this section we basically specify all the files and club then into sensible variable names like source, include, etc. It is just to ease things out, but if you wish you can totally skip this section and use the file names directly instead of the variables.
###########################################################
message(STATUS "### Files ###")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")

# Find all source files
file(GLOB_RECURSE CPLUSPLUS_SOCKET_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
message(STATUS "CPLUSPLUS_SOCKET_SOURCES=${CPLUSPLUS_SOCKET_SOURCES}")

# Find all header files
file(GLOB_RECURSE CPLUSPLUS_SOCKET_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h" )
message(STATUS "CPLUSPLUS_SOCKET_HEADERS=${CPLUSPLUS_SOCKET_HEADERS}")

###########################################################
# target
# This is the part where we tell CMake the name of the output file, in our case we wish to name it as binary. Whatever files names follow after that are basically your source files same way as you do while compiling them manually.
###########################################################
message(STATUS "### Target ###")
# defines our socket binary with all linked source files.
add_executable(socket ${CPLUSPLUS_SOCKET_HEADERS} ${CPLUSPLUS_SOCKET_SOURCES})
# set version number
# This will give you a libbar.so => libbar.so.0 => libbar.so.0.0.0 symlink chain with a SONAME of libbar.so.0 encoded into the library.
# SOVERSION will only be applied to libraries.
set_target_properties(socket PROPERTIES
  VERSION ${PROJECT_SOCKET_VERSION}
  SOVERSION ${PROJECT_SOCKET_SOVERSION})

if (WIN32)
  # set stuff for windows
  target_link_libraries(socket ws2_32)
endif()

###########################################################
# include
# This command is used to specify the path of the include directories that you want the compiler to look into while searching for header files while compiling your code. This will also include the header files from 3rd party libraries as we have done for Randomize and Logger.
###########################################################
# include of this project
target_include_directories(socket PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)

###########################################################
# install
# The TARGETS form specifies rules for installing targets from a project. There are several kinds of target files that may be installed:
###########################################################
message(STATUS "### Install ###")
install(TARGETS socket RUNTIME DESTINATION bin)