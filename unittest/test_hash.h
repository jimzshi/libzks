#include "libzks.h"

#include "hash.h"
#include "hash_table.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <vector>

extern zks::simlog logger;

inline
void test_hash()
{
    std::chrono::milliseconds dura(1000);
    for (size_t i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(dura);
        zks::Hashcode_base_<32> hash1;
        ZKS_INFO(logger, "hash", "hash1(salt): %08X", hash1.h[0]);
        zks::Hashcode_base_<32> hash2(false);
        ZKS_INFO(logger, "hash", "hash2(no_salt): %08X", hash2.h[0]);
    }
    return;
}

inline
void test_hash128()
{
    std::chrono::milliseconds dura(1000);
    for (size_t i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(dura);
        zks::Hashcode_base_<128> hash1;
        ZKS_INFO(logger, "hash128", "hash1(salt): %s", zks::to_u8string(hash1).c_str());
        zks::Hashcode_base_<128> hash2(false);
        ZKS_INFO(logger, "hash128", "hash2(no_salt): %s", zks::to_u8string(hash2).c_str());
    }
    return;
}

inline
void test_hashcode(bool salt = true)
{
    zks::HashCode32 h(salt);
    ZKS_INFO(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    h += 32;
    ZKS_INFO(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    h += 11.23;
    ZKS_INFO(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    h += "haha";
    ZKS_INFO(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    zks::u8string str(L"halo");
    h += str;
    ZKS_INFO(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());
    zks::HashCode32 h2(salt);
    ZKS_INFO(logger, "hash1", "hash_u8: %s", zks::to_u8string(h2).c_str());
    h += h2;
    ZKS_INFO(logger, "hash1", "hash_u8: %s", zks::to_u8string(h).c_str());

    zks::Hashcode_base_<32, zks::MurmurHash<32>, char> h3(salt);
    ZKS_INFO(logger, "hash1", "hash_u8: %s", zks::to_u8string(h3).c_str());
    h3 += h;
    ZKS_INFO(logger, "hash1", "hash_u8: %s", zks::to_u8string(h3).c_str());

    zks::Hashcode_base_<32, zks::MurmurHash<32>, uint16_t> h4(salt);
    ZKS_INFO(logger, "hash1", "hash_u8: %s", zks::to_u8string(h4).c_str());
}

inline
void test_hashcode_128(bool salt=true)
{
    zks::HashCode128 h(salt);
    ZKS_INFO(logger, "test_hashcode_128", "hash_u8: %s", zks::to_u8string(h).c_str());
    h += 32;
    ZKS_INFO(logger, "test_hashcode_128", "hash_u8 + 32: %s", zks::to_u8string(h).c_str());
    h += 11.23;
    ZKS_INFO(logger, "test_hashcode_128", "hash_u8 + 11.23: %s", zks::to_u8string(h).c_str());
    h += "haha";
    ZKS_INFO(logger, "test_hashcode_128", "hash_u8 + haha: %s", zks::to_u8string(h).c_str());
    zks::u8string str(L"halo");
    h += str;
    ZKS_INFO(logger, "test_hashcode_128", "hash_u8 + u8string(halo): %s", zks::to_u8string(h).c_str());
    zks::HashCode128 h2(salt);
    ZKS_INFO(logger, "test_hashcode_128", "hash_u8(h2): %s", zks::to_u8string(h2).c_str());
    h += h2;
    ZKS_INFO(logger, "test_hashcode_128", "hash_u8(h+h2): %s", zks::to_u8string(h).c_str());

    zks::Hashcode_base_<128, zks::MurmurHash<128>, char> h3(salt);
    ZKS_INFO(logger, "test_hashcode_128", "hash_u8(h3<char>): %s", zks::to_u8string(h3).c_str());
    h3 += h;
    ZKS_INFO(logger, "test_hashcode_128", "hash_u8: %s", zks::to_u8string(h3).c_str());

    zks::Hashcode_base_<128, zks::MurmurHash<128>, uint16_t> h4(salt);
    ZKS_INFO(logger, "test_hashcode_128", "hash_u8(h4<uint16_t): %s", zks::to_u8string(h4).c_str());
}

inline
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
    std::vector<zks::HashVector<zks::u8string>> field_symbols{ field_size };
    size_t num=0;
    for (; zks::getline(ifs, line); ++num) {
        items = line.split();
        if(items.size() != field_size) {
            ZKS_INFO(logger, "hashvector", "line(%d) : `%s` failed. \nread in %d items, need %d.", num, line.c_str(), items.size(), field_size);
        }
        //if(field_symbols.contains(items[field_num])) {
        //    ZKS_INFO(logger, "hashvector", "item `%s` appears more than once.", items[field_num].c_str());
        //}
        for (size_t i = 0; i < field_size; ++i) {
            field_symbols[i].push_back(items[i]);
        }
        
        if (!(num & 0xffff)) {
            ZKS_INFO(logger, "hashvector", "read in %d lines.", num);
        }
    }

    ZKS_INFO(logger, "hashvector", "read in %d lines in total", num);
    for (size_t i = 0; i < field_size; ++i) {
        ZKS_INFO(logger, "hashvector", "field `%s` has %d symbols", field_names[i].c_str(), field_symbols[i].size());
    }
    
    //for(size_t i=0; i<field_symbols.size(); ++i) {
    //    ZKS_INFO(logger, "hashvector", "index(%d): %s", i, field_symbols[i].c_str());
    //}

    //char c = std::getchar();

    return ;
}

inline
void test_hash_table(int argc, char* argv[])
{
    if (argc < 5) {
        return;
    }
    std::ifstream ifs(argv[3]);
    int field_num = std::atoi(argv[4]);
    if (field_num < 0) {
        return;
    }

    zks::u8string line;
    zks::getline(ifs, line);
    std::vector<zks::u8string> field_names = line.trim_spaces().split(true, "\t");
    size_t field_size = field_names.size();
    if (field_size < size_t(field_num)) {
        return;
    }

    std::vector<zks::u8string> items;
    std::vector<zks::Hash_table_<zks::u8string>> field_symbols{ field_size };
    size_t num = 0;
    for (; zks::getline(ifs, line); ++num) {
        items = line.split(true, "\t");
        if (items.size() > field_size) {
            ZKS_INFO(logger, "hashvector", "line(%d) : `%s` failed. \nread in %d items, need %d.", num, line.c_str(), items.size(), field_size);
            continue;
        }
        //if(field_symbols.contains(items[field_num])) {
        //    ZKS_INFO(logger, "hashvector", "item `%s` appears more than once.", items[field_num].c_str());
        //}
        for (size_t i = 0; i < items.size(); ++i) {
            field_symbols[i].insert(items[i]);
        }

        if (!(num & 0xffff)) {
            ZKS_INFO(logger, "hashvector", "read in %d lines.", num);
        }
    }

    ZKS_INFO(logger, "hashvector", "read in %d lines in total", num);
    for (size_t i = 0; i < field_size; ++i) {
        ZKS_INFO(logger, "hashvector", "field `%s` has %d symbols", field_names[i].c_str(), field_symbols[i].size());
    }

    //for(size_t i=0; i<field_symbols.size(); ++i) {
    //    ZKS_INFO(logger, "hashvector", "index(%d): %s", i, field_symbols[i].c_str());
    //}

    //char c = std::getchar();

    return;
}