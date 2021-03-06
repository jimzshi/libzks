#include "libzks.h"
#include "bit_hack.h"

extern zks::simlog g_logger;

inline
void test_permutations()
{
    size_t cardinals[5] = { 1, 2, 3, 4, 5 };
    using Perm = zks::Permutations < 5 >;
    Perm perm { cardinals };
    for (size_t i = 0; i < perm.size(); ++i, perm.next()) {
        ZKS_INFO(g_logger, "perm", "%5d/%5d: {%d, %d, %d, %d, %d}", perm.tellp(), perm.size(), perm.index(0), perm.index(1), perm.index(2), perm.index(3),
                perm.index(4));
    }
    perm.seekp(13);
    for (size_t i = 0; i < perm.size(); ++i, perm.prev()) {
        ZKS_INFO(g_logger, "perm", "%5d/%5d: {%d, %d, %d, %d, %d}", perm.tellp(), perm.size(), perm.index(0), perm.index(1), perm.index(2), perm.index(3),
                perm.index(4));
    }
    perm.seekp(26);
    for (size_t i = 0; i < perm.size(); ++i, perm.next()) {
        ZKS_INFO(g_logger, "perm", "%5d/%5d: {%d, %d, %d, %d, %d}", perm.tellp(), perm.size(), perm.index(0), perm.index(1), perm.index(2), perm.index(3),
                perm.index(4));
    }
    ZKS_INFO(g_logger, "perm", "%s", "copy-ctor");
    Perm p2 = perm;
    ZKS_INFO(g_logger, "perm", "p2: %5d/%5d: {%d, %d, %d, %d, %d}", p2.tellp(), p2.size(), p2.index(0), p2.index(1), p2.index(2), p2.index(3), p2.index(4));
    p2.seekp(39);
    ZKS_INFO(g_logger, "perm", "perm: %5d/%5d: {%d, %d, %d, %d, %d}", perm.tellp(), perm.size(), perm.index(0), perm.index(1), perm.index(2), perm.index(3),
            perm.index(4));
    ZKS_INFO(g_logger, "perm", "p2: %5d/%5d: {%d, %d, %d, %d, %d}", p2.tellp(), p2.size(), p2.index(0), p2.index(1), p2.index(2), p2.index(3), p2.index(4));

    return;
}

inline
void test_mac()
{
    std::vector<zks::u8string> mac_addrs = zks::get_mac_address();
    for (size_t i = 0; i < mac_addrs.size(); ++i) {
        ZKS_INFO(g_logger, "utility", "mac[%d]: %s", i, mac_addrs[i].c_str());
    }
    return;
}

