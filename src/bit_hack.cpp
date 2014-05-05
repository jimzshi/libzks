#include "bit_hack.h"

namespace zks
{
#ifdef _ZKS32
    size_t first_bit1(uint32_t x) {
        if (x == 0) {
            return size_t(-1);
        }
        size_t n = 0;
        if (x <= 0xffff) {
            n += 16; x <<= 16;
        }
        if (x <= 0xffffff) {
            n += 8; x <<= 8;
        }
        if (x <= 0xfffffff) {
            n += 4; x <<= 4;
        }
        if (x <= 0x3fffffff) {
            n += 2; x <<= 2;
        }
        if (x <= 0x7fffffff) {
            n += 1;
        }
        return n;
    }
    size_t last_bit1(uint32_t x) {
        if (x == 0) {
            return size_t(-1);
        }
        size_t n = 0;
        if ((x & 0xffff) == 0) {
            n += 16; x >>= 16;
        }
        if ((x & 0xff) == 0) {
            n += 8; x >>= 8;
        }
        if ((x & 0xf) == 0) {
            n += 4; x >>= 4;
        }
        if ((x & 0x3) == 0) {
            n += 2; x >>= 2;
        }
        if ((x & 0x1) == 0) {
            n += 1;
        }
        return 31 - n;
    }
#elif defined(_ZKS64)
    size_t first_bit1(uint64_t x)
    {
        if (x == 0) {
            return size_t(-1);
        }
        size_t n = 0;
        if (x <= 0xffffffff) {
            n += 32;
            x <<= 32;
        }
        if (x <= 0xffffffffffff) {
            n += 16;
            x <<= 16;
        }
        if (x <= 0xffffffffffffff) {
            n += 8;
            x <<= 8;
        }
        if (x <= 0xfffffffffffffff) {
            n += 4;
            x <<= 4;
        }
        if (x <= 0x3fffffffffffffff) {
            n += 2;
            x <<= 2;
        }
        if (x <= 0x7fffffffffffffff) {
            n += 1;
        }
        return n;
    }
    size_t last_bit1(uint64_t x)
    {
        if (x == 0) {
            return size_t(-1);
        }
        size_t n { 0 };
        if ((x & 0xffffffff) == 0) {
            n += 32;
            x >>= 32;
        }
        if ((x & 0xffff) == 0) {
            n += 16;
            x >>= 16;
        }
        if ((x & 0xff) == 0) {
            n += 8;
            x >>= 8;
        }
        if ((x & 0xf) == 0) {
            n += 4;
            x >>= 4;
        }
        if ((x & 0x3) == 0) {
            n += 2;
            x >>= 2;
        }
        if ((x & 0x1) == 0) {
            n += 1;
        }
        return 63 - n;
    }
#endif
}
