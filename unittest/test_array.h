#ifndef TEST_ARRAY_H_
#define TEST_ARRAY_H_

#include "libzks.h"
#include "bit_hack.h"
#include "array.h"
#include "random.h"

#include <typeinfo>

extern zks::simlog logger;
extern zks::RangedRNGen<int> default_rng;

struct ClassTracer {
    int num;
    ClassTracer() : num(default_rng()) {
        ZKS_TRACE(logger, "ClassTracer", "Ctor(%d) - Default", num);
    }
    ClassTracer(int n) : num(n) {
        ZKS_TRACE(logger, "ClassTracer", "Ctor(%d) - Assign", num);
    }
    ClassTracer(ClassTracer const& rh) : num(rh.num) {
        ZKS_TRACE(logger, "ClassTracer", "Ctor(%d) - Copy", num);
    }
    ClassTracer(ClassTracer&& rh) : num(rh.num) {
        rh.num = -1;
        ZKS_TRACE(logger, "ClassTracer", "Ctor(%d) - Move", num);
    }
    ~ClassTracer() {
        ZKS_TRACE(logger, "ClassTracer", "~Detor(%d)", num);
        num = -2;
    }
    ClassTracer& operator=(ClassTracer const& rh) {
        ZKS_TRACE(logger, "ClassTracer", "Operator=(%d) - Copy(%d)", num, rh.num);
        num = rh.num;
        return *this;
    }
    ClassTracer& operator=(ClassTracer&& rh) {
        ZKS_TRACE(logger, "ClassTracer", "Operator=(%d) - Move(%d)", num, rh.num);
        num = rh.num;
        rh.num = -1;
        return *this;
    }
};

inline
bool operator==(ClassTracer const& lh, ClassTracer const& rh) {
    return lh.num == rh.num;
}
inline
bool operator<(ClassTracer const& lh, ClassTracer const& rh) {
    return lh.num < rh.num;
}

template<typename T1_, size_t N1_, size_t N2_, typename T2_>
inline
void log_chunk_array(const char* name, zks::ChunkArray<T1_, N1_, N2_, T2_> const& ca)
{
    ZKS_INFO(logger, "array", "array(%s): size=%d|capacity=%d|chunks=%d|chunk_size=%d|chunk_bytes=%d", name, ca.size(), ca.capacity(), ca.chunks(),
            ca.chunk_size(), ca.chunk_bytes());
    return;
}



template<typename T_>
inline
void log_array(const char* name, zks::LazyArray<T_> const& ca)
{
    ZKS_INFO(logger, "Lazy", "LA(%s): ref(%d), size(%d), capacity(%d)", name, ca.ref(), ca.size(), ca.capacity());
    return;
}


#define LOGA(ta, method) do{ ta.method; log_array(#ta"."#method, ta); } while(0)

inline
void test_lazy_array() {
    typedef zks::LazyArray<ClassTracer> TA;
    TA ta1;
    log_array("init ta1", ta1);
    LOGA(ta1, push_back(ClassTracer(333)));
    TA ta2(ta1);
    log_array("init ta2 from copy ta1", ta2);
    LOGA(ta1, resize(7));
    LOGA(ta1, reserve(17));
    ta1.append() = ClassTracer(444);
    log_array("ta1 append()", ta1);
    LOGA(ta1, erase(6, 1));
    LOGA(ta1, erase(1, 3));
    LOGA(ta1, insert_at(32, ClassTracer(555)));

    log_array("ta2 again", ta2);
    ta2 = ta1;
    log_array("ta2", ta2);

    LOGA(ta1, clear());
    LOGA(ta1, shrink_to_fit());
    LOGA(ta2, clear());
}

inline
void test_chunk_array()
{
    typedef zks::ChunkArray<int> chunk_array_t;
    ZKS_INFO(logger, "array", "sizeof(zks::ChunkArray<int, 10>): %d", sizeof(chunk_array_t));
    ZKS_INFO(logger, "array", "sizeof(zks::LazyArray<int>): %d", sizeof(zks::LazyArray<int>));

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
    ZKS_INFO(logger, "array", "ca2[3]: %s", ca[3].c_str());
    const zks::ChunkArray<zks::u8string, 10> ca2 { ca };
    log_chunk_array("ca2", ca2);
    ZKS_INFO(logger, "array", "ca2[3]: %s", ca2[3].c_str());

    return;
}

inline void test_ca_push()
{

}

#endif /* TEST_ARRAY_H_ */
