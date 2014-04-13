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

	return;
}



#endif /* TEST_ARRAY_H_ */
