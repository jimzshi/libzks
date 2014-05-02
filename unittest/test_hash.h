#include "libzks.h"
#include "hash.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>

extern zks::simlog logger;

void test_hash()
{
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

void test_hash1()
{
    zks::HashCode32 h;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    h += 32;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    h += 11.23;
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    h += "haha";
    ZKS_ERROR(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    zks::u8string str(L"halo");
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

void test_hashvector(int argc, char* argv[])
{
    if (argc < 5) {
        return;
    }
    std::ifstream ifs(argv[3]);
    int field_num = std::atoi(argv[4]);
    if(field_num < 0) {
        return ;
    }

    zks::u8string line;
    zks::getline(ifs, line);
    std::vector<zks::u8string> field_names = line.trim_spaces().split();
    size_t field_size = field_names.size();
    if(field_size <= size_t(field_num)) {
        return;
    }

    std::vector<zks::u8string> items;
    zks::Hash_container_<zks::u8string, std::vector<zks::u8string> > field_symbols;
    size_t num=0;
    for (; zks::getline(ifs, line); ++num) {
        items = line.split();
        if(items.size() != field_size) {
            ZKS_ERROR(logger, "hashvector", "line(%d) : `%s` failed. \nread in %d items, need %d.", num, line.c_str(), items.size(), field_size);
        }
        //if(field_symbols.contains(items[field_num])) {
        //    ZKS_ERROR(logger, "hashvector", "item `%s` appears more than once.", items[field_num].c_str());
        //}
        field_symbols.push_back(items[field_num]);
        if (!(num & 0xffff)) {
            ZKS_ERROR(logger, "hashvector", "read in %d lines.", num);
        }
    }

    ZKS_ERROR(logger, "hashvector", "read in %d lines, %d symbols", num, field_symbols.size());
    //for(size_t i=0; i<field_symbols.size(); ++i) {
    //    ZKS_ERROR(logger, "hashvector", "index(%d): %s", i, field_symbols[i].c_str());
    //}

    //char c = std::getchar();

    return ;
}
