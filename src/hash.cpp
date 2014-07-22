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

}
