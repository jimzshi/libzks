#include "libzks.h"

#include "hash.h"
#include "hash_table.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <cassert>

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

template<int NBIT, int HBIT, typename WORDTYPE>
void log_hc(const char* msg, zks::Hashcode_base_ < NBIT, zks::MurmurHash<HBIT>, WORDTYPE > const& hc)
{
    zks::u8string grp;
    grp.format(64, "test_hash_code<%d, MurmurHash<%d>, %d>", NBIT, HBIT, sizeof(WORDTYPE) * 8);
    ZKS_INFO(logger, grp.c_str(), "%s: %s, %ull", msg, zks::to_u8string(hc).c_str(), hc.to_uint64());
    return;
}
template<int NBIT, int HBIT = NBIT, typename WORDTYPE = uint32_t>
void test_hashcode(bool salt = true)
{
    using hashcode_t = zks::Hashcode_base_ < NBIT, zks::MurmurHash<HBIT>, WORDTYPE > ;

    hashcode_t h(salt);
    log_hc("h", h);
    h += 32;
    log_hc("h+32", h);
    h += 11.23;
    log_hc("h+11.23", h);
    h += "haha";
    log_hc("h+haha", h);
    zks::u8string str(L"halo");
    h += str;
    log_hc("h+str", h);

    hashcode_t h2(salt);
    log_hc("h2", h2);
    h += h2;
    log_hc("h+h2", h);

    zks::Hashcode_base_ < NBIT, zks::MurmurHash<HBIT>, char > h3(salt);
    log_hc("h3", h3);
    h3 += h;
    log_hc("h3+h", h3);

    zks::Hashcode_base_<NBIT, zks::MurmurHash<HBIT>, uint16_t> h4(salt);
    log_hc("h4", h4);
    h4 += h3;
    h4 += h2;
    log_hc("h4 + h2 + h3", h4);

    return;
}

inline
void test_hashvector(int argc, const char* argv[])
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
    std::vector<zks::u8string> field_names = line.trim_spaces().split(true, "\t");
    size_t field_size = field_names.size();
    if(field_size < size_t(field_num)) {
        return;
    }

    std::vector<zks::u8string> items;
    std::vector<zks::HashVector<zks::u8string>> field_symbols{ field_size };
    size_t num=0;

    zks::StopWatch sw;
    sw.start("start");
    for (; zks::getline(ifs, line); ++num) {
        items = line.split(true, "\t");
        if(items.size() > field_size) {
            ZKS_INFO(logger, "hashvector", "line(%d) : `%s` failed. \nread in %d items, need %d.", num, line.c_str(), items.size(), field_size);
            continue;
        }
        //if(field_symbols.contains(items[field_num])) {
        //    ZKS_INFO(logger, "hashvector", "item `%s` appears more than once.", items[field_num].c_str());
        //}
        for (size_t i = 0; i < items.size(); ++i) {
            field_symbols[i].push_back(items[i]);
        }
        
        if (!(num & 0xffff)) {
            ZKS_INFO(logger, "hashvector", "read in %d lines.", num);
        }
    }
    sw.tick("push_back");
    ZKS_INFO(logger, "hashvector", "read in %d lines in total", num);
    for (size_t i = 0; i < field_size; ++i) {
        ZKS_INFO(logger, "hashvector", "field `%s` has %d symbols", field_names[i].c_str(), field_symbols[i].size());
    }
    ifs.clear();
    ifs.seekg(0);
    zks::getline(ifs, line);

    sw.tick("print");
    for (; zks::getline(ifs, line); ++num) {
        items = line.split(true, "\t");
        if (items.size() > field_size) {
            ZKS_INFO(logger, "hash_table", "line(%d) : `%s` failed. \nread in %d items, need %d.", num, line.c_str(), items.size(), field_size);
            continue;
        }
        for (size_t i = 0; i < items.size(); ++i) {
            assert(field_symbols[i].find(items[i]) != size_t(-1));
        }

        if (!(num & 0xffff)) {
            ZKS_INFO(logger, "hashvector", "read in %d lines.", num);
        }
    }
    sw.tick("find");

    ZKS_INFO(logger, "hashvector", "stopwatch: %s", sw.u8str().c_str());

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
	std::vector < zks::Hash_container_< zks::u8string, zks::ChunkArray<zks::u8string>>> field_symbols{ field_size };
    size_t num = 0;
    zks::StopWatch sw;
    sw.start("start");
    for (; zks::getline(ifs, line); ++num) {
        items = line.split(true, "\t");
        if (items.size() > field_size) {
            ZKS_INFO(logger, "hash_table", "line(%d) : `%s` failed. \nread in %d items, need %d.", num, line.c_str(), items.size(), field_size);
            continue;
        }
        for (size_t i = 0; i < items.size(); ++i) {
            field_symbols[i].insert(items[i]);
        }

        if (!(num & 0xffff)) {
            ZKS_INFO(logger, "hashvector", "read in %d lines.", num);
        }
    }
    sw.tick("insert()");

    ZKS_INFO(logger, "hash_table", "read in %d lines in total", num);
    for (size_t i = 0; i < field_size; ++i) {
        ZKS_INFO(logger, "hash_table", "field `%s` has %d symbols", field_names[i].c_str(), field_symbols[i].size());
    }
    ifs.clear();
    ifs.seekg(0);
    zks::getline(ifs, line);

    sw.tick("print");
    for (; zks::getline(ifs, line); ++num) {
        items = line.split(true, "\t");
        if (items.size() > field_size) {
            ZKS_INFO(logger, "hash_table", "line(%d) : `%s` failed. \nread in %d items, need %d.", num, line.c_str(), items.size(), field_size);
            continue;
        }
        for (size_t i = 0; i < items.size(); ++i) {
            assert(field_symbols[i].find(items[i]) != field_symbols[i].end());
        }

        if (!(num & 0xffff)) {
            ZKS_INFO(logger, "hashvector", "read in %d lines.", num);
        }
    }
    sw.tick("find");

    ZKS_INFO(logger, "hash_table", "stopwatch: %s", sw.u8str().c_str());

    return;
}