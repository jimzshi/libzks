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

void test_bitvector() {
    zks::BitVector bv(67);
    bv.set(33);
    ZKS_ERROR(logger, "bitvector", "first_bit1: %d", bv.first_bit1());
    bv.flip();
    ZKS_ERROR(logger, "bitvector", "popcnt: %d", bv.popcnt());
    bv.flip();
    bv.set(17);
    ZKS_ERROR(logger, "bitvector", "first_bit1: %d", bv.first_bit1());
    ZKS_ERROR(logger, "bitvector", "last_bit1: %d", bv.last_bit1());
    ZKS_ERROR(logger, "bitvector", "popcnt: %d", bv.popcnt());
    ZKS_ERROR(logger, "bitvector", "test(%d): %d", 15, bv.test(15));
    ZKS_ERROR(logger, "bitvector", "test(%d): %d", 17, bv.test(17));
    ZKS_ERROR(logger, "bitvector", "test(%d): %d", 33, bv.test(33));
    return;
}

#endif /* TEST_BIT_H_ */
