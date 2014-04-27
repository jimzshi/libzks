#ifndef TEST_ARRAY_H_
#define TEST_ARRAY_H_

#include "libzks.h"
#include "bit_hack.h"
#include "array.h"

#include <typeinfo>

extern zks::simlog logger;

void test_chunk_array() {
	typedef zks::ChunkArray<int, 10> chunk_array_t;
	ZKS_ERROR(logger, "array", "sizeof(zks::ChunkArray<int, 10>): %d", sizeof(chunk_array_t));
	ZKS_ERROR(logger, "array", "sizeof(zks::LazyArray<int>): %d", sizeof(zks::LazyArray<int>));

	zks::ChunkArray<zks::u8string, 10> ca(73);
    ZKS_ERROR(logger, "array", "ca.size(): %d", ca.size());
    ZKS_ERROR(logger, "array", "ca.capacity(): %d", ca.capacity());
    ZKS_ERROR(logger, "array", "ca.chunks(): %d", ca.chunks());
    ZKS_ERROR(logger, "array", "ca.chunk_size(): %d", ca.chunk_size());
    ZKS_ERROR(logger, "array", "ca.chunk_bytes(): %d", ca.chunk_bytes());
    ca.reserve(156);
    ZKS_ERROR(logger, "array", "ca.size(): %d", ca.size());
    ZKS_ERROR(logger, "array", "ca.capacity(): %d", ca.capacity());
    ZKS_ERROR(logger, "array", "ca.chunks(): %d", ca.chunks());
    ZKS_ERROR(logger, "array", "ca.chunk_size(): %d", ca.chunk_size());
    ZKS_ERROR(logger, "array", "ca.chunk_bytes(): %d", ca.chunk_bytes());
    ca.chunk_resize(7);
    ZKS_ERROR(logger, "array", "ca.size(): %d", ca.size());
    ZKS_ERROR(logger, "array", "ca.capacity(): %d", ca.capacity());
    ZKS_ERROR(logger, "array", "ca.chunks(): %d", ca.chunks());
    ZKS_ERROR(logger, "array", "ca.chunk_size(): %d", ca.chunk_size());
    ZKS_ERROR(logger, "array", "ca.chunk_bytes(): %d", ca.chunk_bytes());
    ca.resize(130);
    ZKS_ERROR(logger, "array", "ca.size(): %d", ca.size());
    ZKS_ERROR(logger, "array", "ca.capacity(): %d", ca.capacity());
    ZKS_ERROR(logger, "array", "ca.chunks(): %d", ca.chunks());
    ZKS_ERROR(logger, "array", "ca.chunk_size(): %d", ca.chunk_size());
    ZKS_ERROR(logger, "array", "ca.chunk_bytes(): %d", ca.chunk_bytes());
    ca.reserve(260);
    ZKS_ERROR(logger, "array", "ca.size(): %d", ca.size());
    ZKS_ERROR(logger, "array", "ca.capacity(): %d", ca.capacity());
    ZKS_ERROR(logger, "array", "ca.chunks(): %d", ca.chunks());
    ZKS_ERROR(logger, "array", "ca.chunk_size(): %d", ca.chunk_size());
    ZKS_ERROR(logger, "array", "ca.chunk_bytes(): %d", ca.chunk_bytes());
    ca.shrink_to_fit();
    ZKS_ERROR(logger, "array", "ca.size(): %d", ca.size());
    ZKS_ERROR(logger, "array", "ca.capacity(): %d", ca.capacity());
    ZKS_ERROR(logger, "array", "ca.chunks(): %d", ca.chunks());
    ZKS_ERROR(logger, "array", "ca.chunk_size(): %d", ca.chunk_size());
    ZKS_ERROR(logger, "array", "ca.chunk_bytes(): %d", ca.chunk_bytes());

    ca[3] = "hello";
    ZKS_ERROR(logger, "array", "ca2[3]: %s", ca[3].c_str());
    const zks::ChunkArray<zks::u8string, 10> ca2{ ca };
    ZKS_ERROR(logger, "array", "ca2.size(): %d", ca2.size());
    ZKS_ERROR(logger, "array", "ca2.ca2pacity(): %d", ca2.capacity());
    ZKS_ERROR(logger, "array", "ca2.chunks(): %d", ca2.chunks());
    ZKS_ERROR(logger, "array", "ca2.chunk_size(): %d", ca2.chunk_size());
    ZKS_ERROR(logger, "array", "ca2.chunk_bytes(): %d", ca2.chunk_bytes());

    ZKS_ERROR(logger, "array", "ca2[3]: %s", ca2[3].c_str());

	return;
}

void test_ca_push() {

}


#endif /* TEST_ARRAY_H_ */
