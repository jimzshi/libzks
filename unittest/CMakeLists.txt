include_directories (${LIBZKS_SOURCE_DIR}/src)
link_directories (${LIBZKS_BINARY_DIR}/src)

add_executable (test test.cpp)

set( LD_LIBS ${LD_LIBS} zks)

if(UNIX)
set( LD_LIBS ${LD_LIBS} ${CMAKE_THREAD_LIBS_INIT} c++)
endif(UNIX)

target_link_libraries (test ${LD_LIBS})