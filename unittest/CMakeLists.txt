file(GLOB ZKS_TEST_SRC "test_*.cpp")

include_directories (${libzks_SOURCE_DIR}/src)
link_directories (${libzks_BINARY_DIR}/src)

set( LD_LIBS ${LD_LIBS} zks)
set( LD_LIBS ${LD_LIBS} ${CMAKE_THREAD_LIBS_INIT})

set( LD_LIBS ${LD_LIBS} $<$<CXX_COMPILER_ID:Clang>:c++>)

link_libraries (zks ${LD_LIBS})

foreach(test_src ${ZKS_TEST_SRC})
    GET_FILENAME_COMPONENT(test_bin ${test_src} NAME_WE)
    string(CONCAT test_hdr ${test_bin} .h)
    add_executable (${test_bin} ${test_src} ${test_hdr} test.cpp)
endforeach(test_src)
