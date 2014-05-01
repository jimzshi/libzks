#include "utility.h"
#include "hash.h"
#include "MurmurHash3.h"

#include <ctime>

namespace zks
{

    template<> MurmurHash<32>::result_type MurmurHash<32>::salt(bool fixed)
    {
        if (fixed) {
            return result_type(0x58132134);
        }
        result_type h;
        std::vector<zks::u8string> mac_addrs = get_mac_address();
        for (size_t i = 0; i < mac_addrs.size(); ++i) {
            MurmurHash3_x86_32((void*) mac_addrs[i].data(), mac_addrs[i].size(), h, (void*) &h);
        }
        std::time_t now = std::time(nullptr);
        char* nowstr = std::ctime(&now);
        MurmurHash3_x86_32((void*) nowstr, std::strlen(nowstr), h, (void*) &h);
        return h;
    }
    template<> const MurmurHash<32>::result_type MurmurHash<32>::SALT = MurmurHash<32>::salt();
    template<> MurmurHash<32>::result_type MurmurHash<32>::hash(const void* key, int n, result_type seed)
    {
        result_type h;
        MurmurHash3_x86_32(key, n, SALT, (void*) &h);
        return h;
    }

}
