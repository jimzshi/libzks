#include "utility.h"
#include "hash.h"
#include "MurmurHash3.h"

#include <ctime>

namespace zks
{
    //32bits
    template<> MurmurHash<32>::result_type MurmurHash<32>::salt(bool fixed)
    {
        if (fixed) {
            return result_type(0x58132134);
        }
        result_type h = 0;
        std::vector<zks::u8string> mac_addrs = get_mac_address();
        for (size_t i = 0; i < mac_addrs.size(); ++i) {
            MurmurHash3_x86_32((void*) mac_addrs[i].data(), (int)mac_addrs[i].size(), h, (void*) &h);
        }
        std::time_t now = std::time(nullptr);
        char* nowstr = std::ctime(&now);
        MurmurHash3_x86_32((void*) nowstr, (int)std::strlen(nowstr), h, (void*) &h);
        return h;
    }
    template<> const MurmurHash<32>::result_type MurmurHash<32>::SALT = MurmurHash<32>::salt();
    template<> MurmurHash<32>::result_type MurmurHash<32>::hash(const void* key, size_t n, result_type seed)
    {
        result_type h;
        MurmurHash3_x86_32(key, (int)n, seed, (void*)&h);
        return h;
    }

    //64bits
    template<> MurmurHash<64>::result_type MurmurHash<64>::salt(bool fixed)
    {
        if (fixed) {
            return result_type(0x5813213482463167);
        }
        result_type h = 0;
        uint32_t* p = (uint32_t*)&h;
        std::vector<zks::u8string> mac_addrs = get_mac_address();
        for (size_t i = 0; i < mac_addrs.size(); ++i) {
            MurmurHash3_x86_32((void*)mac_addrs[i].data(), (int)mac_addrs[i].size(), *p, (void*)p);
        }
        std::time_t now = std::time(nullptr);
        char* nowstr = std::ctime(&now);
        MurmurHash3_x86_32((void*)nowstr, (int)std::strlen(nowstr), p[0], (void*)&p[1]);
        return h;
    }
    template<> const MurmurHash<64>::result_type MurmurHash<64>::SALT = MurmurHash<32>::salt();
    template<> MurmurHash<64>::result_type MurmurHash<64>::hash(const void* key, size_t n, result_type seed)
    {
        uint64_t res[2];
        uint32_t* s = (uint32_t*)&seed;
        MurmurHash3_x64_128(key, (int)n, *s, (void*)res);
        return res[0];
    }

    //128bit;
    template<> MurmurHash<128>::result_type MurmurHash<128>::salt(bool fixed)
    {
        if (fixed) {
            result_type ret;
            uint32_t* p = (uint32_t*)&ret;
            (*p++) = 0x58132134;
            (*p++) = 0x94827513;
            (*p++) = 0x16574893;
            (*p)   = 0x17932864;
            return ret;
        }

        result_type h;
        uint32_t* p0 = (uint32_t*)&h;
        std::time_t now = std::time(nullptr);
        char* nowstr = std::ctime(&now);
        MurmurHash3_x86_32((void*)nowstr, (int)std::strlen(nowstr), 0, (void*)p0);

        std::vector<zks::u8string> mac_addrs = get_mac_address();
        for (size_t i = 0; i < mac_addrs.size(); ++i) {
#ifdef _ZKS64
            MurmurHash3_x64_128((void*)mac_addrs[i].data(), (int)mac_addrs[i].size(), *p0, (void*)&h);
#else
            MurmurHash3_x86_128((void*)mac_addrs[i].data(), (int)mac_addrs[i].size(), *p0, (void*)&h);
#endif
        }
        return h;
    }
    template<> const MurmurHash<128>::result_type MurmurHash<128>::SALT = MurmurHash<128>::salt();
    template<> MurmurHash<128>::result_type MurmurHash<128>::hash(const void* key, size_t n, result_type seed)
    {
        result_type h;
#ifdef _ZKS64
        MurmurHash3_x64_128(key, (int)n, *((uint32_t*)&seed), (void*)&h);
#else
        MurmurHash3_x86_128(key, (int)n, *((uint32_t*)&seed), (void*)&h);
#endif
        return h;
    }

    //256bit
    template<> MurmurHash<256>::result_type MurmurHash<256>::salt(bool fixed)
    {
        if (fixed) {
            result_type ret;
            uint32_t* p = (uint32_t*)&ret;
            (*p++) = 0x19876254;
            (*p++) = 0xbadaccee;
            (*p++) = 0xcdcdcdcd;
            (*p++) = 0xabcdef07;

            (*p++) = 0x58132134;
            (*p++) = 0x94827513;
            (*p++) = 0x16574893;
            (*p) = 0x17932864;
            return ret;
        }

        result_type h;
        uint32_t* p0 = (uint32_t*)&h;
        std::time_t now = std::time(nullptr);
        char* nowstr = std::ctime(&now);
        uint32_t seed;
        MurmurHash3_x86_32((void*)nowstr, (int)std::strlen(nowstr), 0, (void*)&seed);

        std::vector<zks::u8string> mac_addrs = get_mac_address();
        for (size_t i = 0; i < mac_addrs.size(); ++i) {
#ifdef _ZKS64
            MurmurHash3_x64_128((void*)mac_addrs[i].data(), (int)mac_addrs[i].size(), seed, (void*)p0);
#else
            MurmurHash3_x86_128((void*)mac_addrs[i].data(), (int)mac_addrs[i].size(), seed, (void*)p0);
#endif
        }
        std::advance(p0, 4);  //next 128;
        for (size_t i = 0; i < mac_addrs.size(); ++i) {
#ifdef _ZKS64
            MurmurHash3_x64_128((void*)mac_addrs[i].data(), (int)mac_addrs[i].size(), seed, (void*)p0);
#else
            MurmurHash3_x86_128((void*)mac_addrs[i].data(), (int)mac_addrs[i].size(), seed, (void*)p0);
#endif
        }
        return h;
    }
    template<> const MurmurHash<256>::result_type MurmurHash<256>::SALT = MurmurHash<256>::salt();
    template<> MurmurHash<256>::result_type MurmurHash<256>::hash(const void* key, size_t n, result_type seed)
    {
        result_type h;
        uint32_t* ph = (uint32_t*)&h;
        uint32_t* ps = (uint32_t*)&seed;
#ifdef _ZKS64
        MurmurHash3_x64_128(key, (int)n, *(ps), (void*)ph);
        ++ps; ph += 4;
        MurmurHash3_x64_128(key, (int)n, *(ps), (void*)ph);
#else
        MurmurHash3_x32_128(key, (int)n, *(ps), (void*)ph);
        ++ps; ph += 4;
        MurmurHash3_x32_128(key, (int)n, *(ps), (void*)ph);
#endif
        return h;
    }
}
