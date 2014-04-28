#ifndef TEST_ARRAY_H_
#define TEST_ARRAY_H_

#include "libzks.h"
#include "bit_hack.h"
#include "array.h"

#include <typeinfo>

extern zks::simlog logger;

template<typename T1_, int N1_, int N2_, typename T2_>
void log_chunk_array(const char* name, zks::ChunkArray<T1_, N1_, N2_, T2_> const& ca) {
	ZKS_ERROR(logger, "array", "array(%s): size=%d|capacity=%d|chunks=%d|chunk_size=%d|chunk_bytes=%d",
			name, ca.size(), ca.capacity(), ca.chunks(), ca.chunk_size(), ca.chunk_bytes());
	return;
}
void test_chunk_array() {
	typedef zks::ChunkArray<int, 10> chunk_array_t;
	ZKS_ERROR(logger, "array", "sizeof(zks::ChunkArray<int, 10>): %d", sizeof(chunk_array_t));
	ZKS_ERROR(logger, "array", "sizeof(zks::LazyArray<int>): %d", sizeof(zks::LazyArray<int>));

	zks::ChunkArray<zks::u8string, 10> ca(3);
	log_chunk_array("ca", ca);
	ca.push_back("world");
	log_chunk_array("ca", ca);
    ca.reserve(156);
    log_chunk_array("ca", ca);
    ca.chunk_resize(7);
    log_chunk_array("ca", ca);
    ca.resize(130);
    log_chunk_array("ca", ca);
    ca.reserve(260);
    log_chunk_array("ca", ca);
    ca.shrink_to_fit();
    log_chunk_array("ca", ca);
    ca[3] = "hello";
    ZKS_ERROR(logger, "array", "ca2[3]: %s", ca[3].c_str());
    const zks::ChunkArray<zks::u8string, 10> ca2{ ca };
    log_chunk_array("ca2", ca2);
    ZKS_ERROR(logger, "array", "ca2[3]: %s", ca2[3].c_str());

	return;
}

void test_ca_push() {

}


#endif /* TEST_ARRAY_H_ */
