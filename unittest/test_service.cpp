#include "test.h"
#include "test_service.h"

#include "u8string.h"
#include "json.h"

#include <iostream>
#include <fstream>

using namespace zks;

int main(int argc, const char* argv[]) {
    if (!test_init(argc, argv)) {
        return -1;
    }

    if (!g_settings.has_option("buildit", "goods")) {
        ZKS_ERROR(g_logger, "buildit", "can't find option %s", "buildit");
        return -2;
    }

    u8string goods_json;
    g_settings.option("buildit", "goods", &goods_json);

    std::ifstream goods_ifs(goods_json.c_str(), std::ifstream::binary);
    if (!goods_ifs) {
        ZKS_ERROR(g_logger, "buildit", "can't open file %s", goods_json.c_str());
        return -3;
    }
    Json::Value root;
    goods_ifs >> root;

    std::cout << root;

    return 0;
}

inline WareHouse::WareHouse(std::string const & id, size_t cap) : id_(id), cap_(cap), size_(0) {}

inline std::string const & WareHouse::id() const { return id_; }

inline size_t WareHouse::capacity() const { return cap_; }

size_t WareHouse::idle_size() const
{
    lock_t lock(mutex_);
    return cap_ - size_;
}

int WareHouse::add_goods(GoodsPtr && g)
{
    if (!g || g->state != Goods::State::DONE) {
        return -1;
    }
    lock_t lock(mutex_);
    if (idle_size() > 0) {
        goods_map_[g->name].emplace_back(move(g));
        return (int)++size_;
    }
    return -2;
}

size_t WareHouse::goods_count(std::string const & name) const
{
    lock_t lock(mutex_);
    return goods_map_.count(name) ? goods_map_.at(name).size() : 0;
}

inline Factory::Factory(std::string const & id, size_t s) : id_(id), prod_queue(s) {}

inline std::string const & Factory::id() const { return id_; }

inline size_t Factory::size() const { return prod_queue.size(); }

size_t Factory::idle_size() const
{
    lock_t lock(mutex_);
    return count_if(prod_queue.cbegin(), prod_queue.cend(),
        std::logical_not<GoodsPtr>());
}

int Factory::add_goods(GoodsPtr && p)
{
    if (!p || (p->state != Goods::State::INIT)) {
        return -1;
    }
    lock_t lock(mutex_);
    int i = 0;
    for (; i < prod_queue.size(); ++i) {
        if (!prod_queue[i]) {
            prod_queue[i] = move(p);
            return i;
        }
    }
    return -2;
}
