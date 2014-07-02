cmake_minimum_required (VERSION 2.6) 
project(LIBZKS)

add_compile_options ($<$<CXX_COMPILER_ID:Clang>:-std=c++1y>)
add_compile_options ($<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++>)

#set( CMAKE_VERBOSE_MAKEFILE on )

add_subdirectory (src)
add_subdirectory (unittest)