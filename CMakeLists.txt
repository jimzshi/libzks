#CC=clang CXX=clang++ cmake -DCMAKE_INSTALL_PREFIX:PATH=/home/zks/local/ ../libzks
cmake_minimum_required (VERSION 2.6) 
project(libzks)

if(NOT CMAKE_BUILD_TYPE)
    set( CMAKE_BUILD_TYPE Release )
endif()

add_compile_options ($<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-std=c++1y>)
add_compile_options ($<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++>)

string( TOLOWER ${CMAKE_BUILD_TYPE} CONFIG )
set( CMAKE_DEBUG_POSTFIX "d" )

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	string(CONCAT CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " -lc++")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	string(CONCAT CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " -framework CoreFoundation -framework IOKit")
endif()

string(CONCAT CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} " -DDEBUG -D_DEBUG")
string(CONCAT CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} " -D_NDEBUG")

if(MSVC)
    option(MSVC_LINK_STATIC_LIB "use /MT(d) for code generation." on)
    if(MSVC_LINK_STATIC_LIB)
        string(CONCAT CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS")
        foreach(CompilerFlag
                CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
                CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
        if(${CompilerFlag} MATCHES "/MD")
                string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
        endif(${CompilerFlag} MATCHES "/MD")
        endforeach(CompilerFlag)
    endif()
endif()

FIND_PATH(CXXTEST_INCLUDE_DIR
  cxxtest/TestSuite.h
  PATHS
  "$ENV{HOME}/local/include/"
  /usr/include/
  /usr/local/include/
  #MSVC
  "$ENV{UserProfile}/local/include/"
  "$ENV{LIB_DIR}/include"
  $ENV{CXXTEST_INCLUDE_PATH}
  #mingw
  c:/msys/local/include
  )
find_package(CxxTest)
if(CXXTEST_FOUND)
    message(STATUS "CxxTest Found: ${CXXTEST_INCLUDE_DIR}")
else (CXXTEST_FOUND)
    message(WARNING, "CxxTest NOT Found.")
endif()

MESSAGE(STATUS "CONFIG : ${CONFIG}")
MESSAGE(STATUS "CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "CMAKE_CXX_COMPILER_ID : ${CMAKE_CXX_COMPILER_ID}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS : ${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS_DEBUG : ${CMAKE_CXX_FLAGS_DEBUG}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS_RELEASE : ${CMAKE_CXX_FLAGS_RELEASE}")
MESSAGE(STATUS "CMAKE_STANDARD_LIBRARIES : ${CMAKE_STANDARD_LIBRARIES}")
MESSAGE(STATUS "CMAKE_EXE_LINKER_FLAGS : ${CMAKE_EXE_LINKER_FLAGS}")
MESSAGE(STATUS "CMAKE_DEBUG_POSTFIX : ${CMAKE_DEBUG_POSTFIX}")

set( CMAKE_DEBUG_TARGET_PROPERTIES on )
set( CMAKE_VERBOSE_MAKEFILE on )

find_package ( Threads REQUIRED )

add_subdirectory (src)
add_subdirectory (unittest)
add_subdirectory (xunit)
