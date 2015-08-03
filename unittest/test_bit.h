#ifndef TEST_BIT_H_
#define TEST_BIT_H_

#include "test.h"

#include <algorithm>
#include <numeric>

inline
void test_bit()
{
    ZKS_INFO(g_logger, "bithack", "next_power_of_2<4096>: %d", zks::NextPowerOf2<unsigned, 4096>::value);
    //ZKS_INFO(logger, "bithack", "next_power_of_2<0>: %d", zks::NextPowerOf2<unsigned, 0>::value);
    ZKS_INFO(g_logger, "bithack", "next_power_of_2<23>: %d", zks::NextPowerOf2<unsigned, 23>::value);
    ZKS_INFO(g_logger, "bithack", "next_power_of_2<13>: %d", zks::NextPowerOf2<int, 13>::value);

    ZKS_INFO(g_logger, "bithack", "logbase2<13>: %d", zks::LogBase2<int, 13>::value);
    ZKS_INFO(g_logger, "bithack", "logbase2<1>: %d", zks::LogBase2<int, 1>::value);
    ZKS_INFO(g_logger, "bithack", "logbase2<0>: %d", zks::LogBase2<int, 0>::value);
    ZKS_INFO(g_logger, "bithack", "logbase2<4096>: %d", zks::LogBase2<int, 4096>::value);

    ZKS_INFO(g_logger, "bithack", "is_pointer: %d", std::is_pointer<int[4]>::value);
    ZKS_INFO(g_logger, "bithack", "is_pointer: %d", std::is_array<int[4]>::value);
    return;
}

inline
void log_bv(zks::BitVector const& bv) {
    ZKS_INFO(g_logger, "bitvector", "BitVec(%s), size(%d), popcnt(%d), first_bit1(%d), last_bit1(%d), ",
            bv.to_u8string().c_str(), bv.size(), bv.popcnt(), bv.first_bit1(), bv.last_bit1());
}

inline
void test_bitvector() {
    zks::BitVector bv(67, false);
    bv.set(33);
    log_bv(bv);
    bv.flip();
    log_bv(bv);
    bv.flip();
    log_bv(bv);
    bv.set(17);
    log_bv(bv);

    std::vector<size_t> indices(10);
    size_t pos = { 10 };
    std::generate(indices.begin(), indices.end(), [&](){return pos += 4; });
    bv.set(indices.begin(), indices.end());
    log_bv(bv);
    for (size_t p{ bv.first_bit1() }, end{ bv.last_bit1() }; p <= end; p = bv.next_bit1(p)) {
        ZKS_INFO(g_logger, "bitvector", "traverse all set bits: %d", p);
    }

    ZKS_INFO(g_logger, "bitvector", "%s", "another traverse method: ");
    indices.clear();
    bv.get_indices1(indices);
    for (auto i : indices) {
		g_logger << i << ", ";
    }
	g_logger << "\n";

    bv.reset(indices.begin(), indices.end());
    log_bv(bv);

    return;
}

inline
void test_bitvector_perf() {
    zks::StopWatch sw;
    sw.start();

    zks::BitVector bv(60000000, false);
    std::vector<size_t> indices(30000000);
    sw.tick("init.");

    zks::randomize();
    zks::sample(indices.begin(), indices.end(), zks::RangedRNGen<size_t>(0, 59999999));
    sw.tick("sample");

    bv.set(indices.begin(), indices.end());
    sw.tick("bv.set 30M");

    size_t cnt = bv.popcnt();
    sw.tick("bv.popcnt()");

    for (size_t p{ bv.first_bit1() }, end{ bv.last_bit1() }; p <= end; p = bv.next_bit1(p)) {
    }
    sw.tick("traverse set bits.");

    bv.reset(indices.begin(), indices.end());
    sw.tick("reset all set bits.");

    ZKS_INFO(g_logger, "bitvector", "count: %d, %s", cnt, sw.u8str().c_str());
    return;
}

#endif /* TEST_BIT_H_ */
