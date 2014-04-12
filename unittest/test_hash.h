#include "libzks.h"
#include "hash.h"
#include <chrono>
#include <thread>

extern zks::simlog logger;

void test_hash() {
    for (size_t i = 0; i < 5; ++i) {
        std::chrono::milliseconds dura(1000);
        std::this_thread::sleep_for(dura);
        zks::Hashcode_base_<32> hash1;
        ZKS_ERROR(logger, "hash", "hash1(salt): %08X", hash1.h[0]);
        zks::Hashcode_base_<32> hash2(false);
        ZKS_ERROR(logger, "hash", "hash2(no_salt): %08X", hash2.h[0]);
    }
    return;
}

void test_hash1() {
    zks::HashCode32 h;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    h += 32;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    h += 11.23;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    h += "haha";
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    zks::u8string str(L"¹þ¹þÄãºÃÂð");
    h += str;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    zks::HashCode32 h2;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h2).c_str());
    h += h2;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());

    zks::Hashcode_base_<32, zks::MurmurHash<32>, char> h3;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h3).c_str());
    h3 += h;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h3).c_str());

    zks::Hashcode_base_<32, zks::MurmurHash<32>, uint16_t> h4;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h4).c_str());
}