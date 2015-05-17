file(GLOB XTEST_HEADER "*.h")

include_directories (${libzks_SOURCE_DIR} ${libzks_SOURCE_DIR}/src)
link_directories (${libzks_BINARY_DIR}/src)

set( LD_LIBS ${LD_LIBS} zks)
set( LD_LIBS ${LD_LIBS} ${CMAKE_THREAD_LIBS_INIT})
set( LD_LIBS ${LD_LIBS} $<$<CXX_COMPILER_ID:Clang>:c++>)

if(CXXTEST_FOUND)
    include_directories(${CXXTEST_INCLUDE_DIR})
    enable_testing()
    CXXTEST_ADD_TEST(xtest xtest.cpp ${XTEST_HEADER})
    target_link_libraries(xtest ${LD_LIBS}) # as needed
endif()
