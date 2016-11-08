#include "service.h"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <algorithm>
#include <unordered_map>

class Goods : std::enable_shared_from_this<Goods> {
public:
    std::string name;
    int prod_time;
    int price;
    enum State { INIT, PROD, DONE, CONSUMED };
    State state;
    Goods(std::string const& n, int t, int p) : name(n), prod_time(t), price(p) { state = INIT; }
    ~Goods() = default;
};

using GoodsPtr = std::unique_ptr<Goods>;
using Gradient = std::unordered_map<std::string, size_t>;
using GoodsTray = std::unordered_map<std::string, std::vector<GoodsPtr>>;

class WareHouse : zks::pattern::Service {
    std::string id_;
    size_t cap_;
    size_t size_;
    GoodsTray goods_map_;
    mutable std::recursive_mutex mutex_;

private:
    void operator=(WareHouse const&) = delete;

private:
    using lock_t = std::lock_guard<std::recursive_mutex>;

public:
    WareHouse(std::string const& id, size_t cap);
    ~WareHouse() {}

    std::string const& id() const;
    size_t capacity() const;
    size_t idle_size() const;

    int add_goods(GoodsPtr&& g);
    size_t goods_count(std::string const& name) const;
};

class Factory : std::enable_shared_from_this<Factory> {
    mutable std::recursive_mutex mutex_;
    std::string id_;
    std::vector<GoodsPtr> prod_queue;

private:
    using lock_t = std::lock_guard<std::recursive_mutex>;

public:
    Factory(std::string const& id, size_t s);
    std::string const& id() const;
    size_t size() const;
    size_t idle_size() const;
    int add_goods(GoodsPtr&& p);
};
