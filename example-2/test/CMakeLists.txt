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
add_executable(${PROJECT_TARGET_TEST} ${PROJECT_HEADERS} ${PROJECT_SOURCES} )
# set version number
set_target_properties(${PROJECT_TARGET_TEST} PROPERTIES VERSION ${PROJECT_VERSION})
# set language
set_target_properties(${PROJECT_TARGET_BINARY} PROPERTIES LINKER_LANGUAGE CXX)

# The COMMAND to run gcov is possible thanks to the -fprofile-arcs -ftest-coverage compile flags
set_target_properties(${PROJECT_TARGET_TEST} PROPERTIES LINK_FLAGS  "-fprofile-arcs -ftest-coverage")
set_target_properties(${PROJECT_TARGET_TEST} PROPERTIES COMPILE_FLAGS  "-fprofile-arcs -ftest-coverage")

###########################################################
# external libs
# This part is what we call linking in compilation terms. So what you have done is you have included the header files of these 3rd party libraries and now you need to tell the compiler where exactly are these libraries located.
###########################################################
target_link_libraries(${PROJECT_TARGET_TEST} PUBLIC ${PROJECT_TARGET_LIBRARY_TEST})




###########################################################
# tests
###########################################################
add_test(NAME ${PROJECT_TARGET_TEST} COMMAND ${PROJECT_TARGET_TEST})
