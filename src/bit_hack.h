#ifndef ZKS_BIT_HACK_H_
#define ZKS_BIT_HACK_H_

#include "configure.h"

#include <type_traits>
#include <cinttypes>
#include <limits>
#include <vector>

namespace zks
{

    template<typename T_, T_ N_, unsigned MAX_ = sizeof(T_) * CHAR_BIT, unsigned CUR_ = 1, typename = typename std::enable_if<std::is_integral<T_>::value>::type>
    struct NextPowerOf2
    {
        static const T_ value = (MAX_ <= CUR_) ? N_ : NextPowerOf2<T_, (((N_ - 1) | ((N_ - 1) >> CUR_)) + 1), MAX_, (CUR_ << 1)>::value;
    };

    template<typename T_, T_ N_, unsigned M_>
    struct NextPowerOf2<T_, N_, M_, M_, void>
    {
        static const T_ value = N_ ? N_ : 1;
    };

    template<typename T_>
    inline T_ next_pow2(T_ x)
    {
        static unsigned maxb = sizeof(T_) * CHAR_BIT;
        --x;
        for (unsigned i = 1; i < maxb; ++i) {
            x |= x >> i;
        }
        return ++x;
    }

    template<typename T_, T_ N_, unsigned R_ = 0, typename = typename std::enable_if<std::is_integral<T_>::value>::type>
    struct LogBase2
    {
        static const T_ value = N_ >> 1 ? LogBase2<T_, (N_ >> 1), (R_ + 1)>::value : R_;
    };

    template<unsigned R_> struct LogBase2<int, 0, R_, void>
    {
        static const int value = 0;
    };

#ifdef _ZKS32
    inline int popcnt(uint32_t x) {
        x = x - ((x >> 1) & 0x55555555);
        x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
        x = ((x + (x >> 4) & 0xf0f0f0f) * 0x1010101) >> 24;
        return (int)x;
    }
    inline int popcnt(uint32_t x, uint32_t pos) {
        if (pos < 32) {
            x = x >> (32 - pos);
        }
        x = x - ((x >> 1) & 0x55555555);
        x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
        x = ((x + (x >> 4) & 0xf0f0f0f) * 0x1010101) >> 24;
        return (int)x;
    }
    /* position counting from MSB, return size_t(-1) if not found */
    size_t first_bit1(uint32_t x);
    /* position counting from MSB, return size_t(-1) if not found */
    size_t last_bit1(uint32_t x);

#elif defined(_ZKS64)
    inline int popcnt(uint64_t x)
    {
        x = x - ((x >> 1) & 0x5555555555555555);
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
        x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;
        x = (x * 0x0101010101010101) >> 56;
        return (int) x;
    }
    inline int popcnt(uint64_t x, uint64_t pos)
    {
        if (pos < 64) {
            x = x >> (64 - pos);
        }
        x = x - ((x >> 1) & 0x5555555555555555);
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
        x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;
        x = (x * 0x0101010101010101) >> 56;
        return (int) x;
    }
    /* position counting from MSB, return size_t(-1) if not found */
    size_t first_bit1(uint64_t x);
    /* position counting from MSB, return size_t(-1) if not found */
    size_t last_bit1(uint64_t x);
#endif

} // namespace zks;

#endif
