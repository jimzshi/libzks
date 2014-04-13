#ifndef TEST_ARRAY_H_
#define TEST_ARRAY_H_

#include "libzks.h"
#include "bit_hack.h"
#include "array.h"

#include <typeinfo>

extern zks::simlog logger;

void test_chunk_array() {
	typedef zks::ChunkArray<int, 10> chunk_array_t;

	zks::ChunkArray<zks::u8string, 10> ca(73);
    ZKS_ERROR(logger, "array", "ca.size(): %d", ca.size());
    ZKS_ERROR(logger, "array", "ca.capacity(): %d", ca.capacity());

    ca.reserve(156);
    ZKS_ERROR(logger, "array", "ca.size(): %d", ca.size());
    ZKS_ERROR(logger, "array", "ca.capacity(): %d", ca.capacity());

	ZKS_ERROR(logger, "array", "sizeof(zks::ChunkArray<int, 10>): %d", sizeof(chunk_array_t));
	ZKS_ERROR(logger, "array", "sizeof(zks::LazyArray<int>): %d", sizeof(zks::LazyArray<int>));

    int* pi = new int(3);
    int* pa = new int[3]{1, 2, 3};
	ZKS_ERROR(logger, "array", "type name of pi: %s", typeid(pi).name());
	ZKS_ERROR(logger, "array", "type name of pa: %s", typeid(pa).name());
	ZKS_ERROR(logger, "array", "type name of int[3]: %s", typeid(int[3]).name());
    delete[] pi;
    delete[] pa;

	return;
}



#endif /* TEST_ARRAY_H_ */
