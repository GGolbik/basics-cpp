# name of the target platform
set(CMAKE_SYSTEM_NAME Windows)
# (and optionally target processor architecture):
set(CMAKE_SYSTEM_PROCESSOR x86)

# path to the toolchain binaries (C compiler, C++ compiler, linker, etc.):
#set(CMAKE_AR <path_to_ar>)
#set(CMAKE_ASM_COMPILER <path_to_assembler>)
#set(CMAKE_C_COMPILER <path_to_c_compiler)
#set(CMAKE_LINKER <path_to_linker>)
#set(CMAKE_OBJCOPY <path_to_objcopy>)
#set(CMAKE_RANLIB <path_to_ranlib>)
#set(CMAKE_SIZE <path_to_size>)
#set(CMAKE_STRIP <path_to_strip>)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++-posix)

# required compilation and linking flags on that particular platform:
#set(CMAKE_C_FLAGS <c_flags>)
# Add "-static" or you need to copy the libstdc++-6.dll library and a few more from your mingw installation (should be in /usr/lib/gcc/x86_64-w64-mingw32/8.3-posix/) 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
#set(CMAKE_C_FLAGS_DEBUG <c_flags_for_debug>)
#set(CMAKE_C_FLAGS_RELEASE <c_flags_for_release>)
#set(CMAKE_CXX_FLAGS_DEBUG <cpp_flags_for_debug>)
#set(CMAKE_CXX_FLAGS_RELEASE <cpp_flags_for_release>)
#set(CMAKE_EXE_LINKER_FLAGS <linker_flags>)