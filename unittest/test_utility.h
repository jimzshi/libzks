#include "libzks.h"
#include "bit_hack.h"

extern zks::simlog logger;

void test_permutations() {
    size_t cardinals[5] = { 1, 2, 3, 4, 5 };
    using Perm = zks::Permutations < 5 > ;
    Perm perm{ cardinals };
    for (size_t i = 0; i < perm.size(); ++i, perm.next()) {
        ZKS_ERROR(logger, "perm", "%5d/%5d: {%d, %d, %d, %d, %d}", perm.tellp(), perm.size(),
            perm.index(0), perm.index(1), perm.index(2), perm.index(3), perm.index(4));
    }
    perm.seekp(13);
    for (size_t i = 0; i < perm.size(); ++i, perm.prev()) {
        ZKS_ERROR(logger, "perm", "%5d/%5d: {%d, %d, %d, %d, %d}", perm.tellp(), perm.size(),
            perm.index(0), perm.index(1), perm.index(2), perm.index(3), perm.index(4));
    }
    perm.seekp(26);
    for (size_t i = 0; i < perm.size(); ++i, perm.next()) {
        ZKS_ERROR(logger, "perm", "%5d/%5d: {%d, %d, %d, %d, %d}", perm.tellp(), perm.size(),
            perm.index(0), perm.index(1), perm.index(2), perm.index(3), perm.index(4));
    }
    ZKS_ERROR(logger, "perm", "%s", "copy-ctor");
    Perm p2 = perm;
    ZKS_ERROR(logger, "perm", "p2: %5d/%5d: {%d, %d, %d, %d, %d}", p2.tellp(), p2.size(),
        p2.index(0), p2.index(1), p2.index(2), p2.index(3), p2.index(4));
    p2.seekp(39);
    ZKS_ERROR(logger, "perm", "perm: %5d/%5d: {%d, %d, %d, %d, %d}", perm.tellp(), perm.size(),
        perm.index(0), perm.index(1), perm.index(2), perm.index(3), perm.index(4));
    ZKS_ERROR(logger, "perm", "p2: %5d/%5d: {%d, %d, %d, %d, %d}", p2.tellp(), p2.size(),
        p2.index(0), p2.index(1), p2.index(2), p2.index(3), p2.index(4));

    return;
}

void test_mac() {
    std::vector<zks::u8string> mac_addrs = zks::get_mac_address();
    for (size_t i = 0; i < mac_addrs.size(); ++i) {
        ZKS_ERROR(logger, "utility", "mac[%d]: %s", i, mac_addrs[i].c_str());
    }
    return;
}


