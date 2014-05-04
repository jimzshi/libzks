#ifndef TEST_BIT_H_
#define TEST_BIT_H_

#include "libzks.h"
#include "bit_hack.h"
#include "array.h"
#include "BitVector.h"

extern zks::simlog logger;

void test_bit()
{
    ZKS_ERROR(logger, "bithack", "next_power_of_2<4096>: %d", zks::NextPowerOf2<unsigned, 4096>::value);
    ZKS_ERROR(logger, "bithack", "next_power_of_2<0>: %d", zks::NextPowerOf2<unsigned, 0>::value);
    ZKS_ERROR(logger, "bithack", "next_power_of_2<23>: %d", zks::NextPowerOf2<unsigned, 23>::value);
    ZKS_ERROR(logger, "bithack", "next_power_of_2<13>: %d", zks::NextPowerOf2<int, 13>::value);

    ZKS_ERROR(logger, "bithack", "logbase2<13>: %d", zks::LogBase2<int, 13>::value);
    ZKS_ERROR(logger, "bithack", "logbase2<1>: %d", zks::LogBase2<int, 1>::value);
    ZKS_ERROR(logger, "bithack", "logbase2<0>: %d", zks::LogBase2<int, 0>::value);
    ZKS_ERROR(logger, "bithack", "logbase2<4096>: %d", zks::LogBase2<int, 4096>::value);

    ZKS_ERROR(logger, "bithack", "is_pointer: %d", std::is_pointer<int[4]>::value);
    ZKS_ERROR(logger, "bithack", "is_pointer: %d", std::is_array<int[4]>::value);
    return;
}

void log_bv(zks::BitVector const& bv) {
    ZKS_ERROR(logger, "bitvector", "BitVec(%s), size(%d), popcnt(%d), first_bit1(%d), last_bit1(%d), ",
            bv.to_u8string().c_str(), bv.size(), bv.popcnt(), bv.first_bit1(), bv.last_bit1());
}
void test_bitvector() {
    zks::BitVector bv(67);
    bv.set(33);
    log_bv(bv);
    bv.flip();
    log_bv(bv);
    bv.flip();
    log_bv(bv);
    bv.set(17);
    log_bv(bv);
    return;
}

#endif /* TEST_BIT_H_ */
