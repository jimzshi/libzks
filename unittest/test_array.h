#ifndef TEST_ARRAY_H_
#define TEST_ARRAY_H_

#include "libzks.h"
#include "bit_hack.h"
#include "array.h"

extern zks::simlog logger;

void test_chunk_array() {
	typedef zks::ChunkArray<int, 10> chunk_array_t;
	zks::ChunkArray<int, 10> ca;
	ca.size();
	ZKS_ERROR(logger, "array", "sizeof(zks::ChunkArray<int, 10>): %d", sizeof(chunk_array_t));
	ZKS_ERROR(logger, "array", "sizeof(zks::LazyArray<int>): %d", sizeof(zks::LazyArray<int>));
	ZKS_ERROR(logger, "array", "sizeof(typename chunk_array_t::Sizetraits_base_): %d",
			sizeof(typename chunk_array_t::Sizetraits_base_));
	return;
}



#endif /* TEST_ARRAY_H_ */
