#ifndef ZKS_HASH_TABLE_H_
#define ZKS_HASH_TABLE_H_

#include "configure.h"
#include "array.h"
#include "hash.h"
#include "utility.h"

#include <mutex>
#include <cassert>

namespace zks {


    class Default_rehash_strategy_ {
        size_t table_size_;
        float r_;
    public:
        Default_rehash_strategy_() : table_size_(3), r_(1.0) {}
        void next() { 
            r_ *= (float)1.3;
            table_size_ = next_prime(2 * table_size_ + 1);
        }
        void prev() { 
            r_ /= (float)1.3; 
            table_size_ = next_prime(table_size_ / 2 | 1);
        }
        size_t table_size() const { return table_size_; }
        size_t bucket_size() const { return (size_t)(3.0 * r_); }
    };

    template<typename T_,
        typename Hash_ = zks::MMHash_base_<T_, 64, false, uint64_t>,
        typename Equal_ = std::equal_to<T_>,
        typename Strategy_ = Default_rehash_strategy_
    >
    class Hash_table_
    {
    public:
        using value_type = T_;
        using hasher = Hash_;
        using equal_type = Equal_;
        using rehash_strategy = Strategy_;

        struct node_type {
            size_t hash_code;
            size_t obj_idx;
        };

    private:
        using mutex_t = std::recursive_mutex;
        using lock_t = std::lock_guard < mutex_t > ;
        using obj_array_t = zks::ChunkArray < value_type >;
        using node_array_t = zks::ChunkArray < node_type >;
        using obj2node_t = zks::ChunkArray < size_t > ;
        //using obj_array_t = std::vector < value_type >;
        //using node_array_t = std::vector < node_type >;
        //using obj2node_t = std::vector < size_t >;
    public:
        using iterator = typename obj_array_t::iterator;
        using const_iterator = typename obj_array_t::const_iterator;
        using res_pair = std::pair < iterator, bool > ; // <iter of obj_array, inserted>;

        mutex_t mutex_;
        hasher hf_;
        equal_type equal_to_;
        rehash_strategy strategy_;
        obj_array_t obj_array_;
        node_array_t table_;
        obj2node_t obj2node_;  // helper of erase();


    public:
        Hash_table_() {
            obj_array_.reserve(strategy_.table_size());
            obj2node_.reserve(strategy_.table_size());
            table_.resize(strategy_.table_size());
        }

        bool empty() const { return obj_array_.size() == 0; }
        size_t size() const { return obj_array_.size(); }
        size_t max_size() const { return strategy_.table_size(); }
        size_t bucket_size() const { return strategy_.bucket_size(); }

        iterator begin() { return obj_array_.begin(); }
        iterator end() { return obj_array_.end(); }
        const_iterator begin() const { return obj_array_.begin(); }
        const_iterator end() const { return obj_array_.end(); }
        const_iterator cbegin() const { return obj_array_.cbegin(); }
        const_iterator cend() const { return obj_array_.cend(); }

        iterator find(const value_type& obj) {
            size_t hc = hash_(obj);
            size_t pos = hc % strategy_.table_size();
            size_t step = 0;
            for (; step < strategy_.bucket_size(); ++step, ++pos) {
                if (pos >= table_.size()) {
                    pos -= table_.size();
                }
                if (table_[pos].hash_code == 0) {
                    return end();
                }
                if (table_[pos].hash_code == hc
                    && equal_to_(obj, obj_array_[table_[pos].obj_idx])) {
                    return iterator(table_[pos].obj_idx, &obj_array_);
                    //return obj_array_.begin() + table_[pos].obj_idx;
                }
            }
            return end();
        }

        bool rehash() {
            lock_t here(mutex_);
            strategy_.next();
            node_array_t new_table(max_size());
            obj2node_t new_obj2node(obj_array_.size());
            
            node_type n;
            for (size_t i = 0; i < obj_array_.size(); ++i) {
                n = table_[obj2node_[i]];
                if (!insert_node_(std::move(n), &new_table, &new_obj2node)) {
                    return false;
                }
            }

            obj_array_.reserve(max_size());
            obj2node_.reserve(max_size());
            table_ = std::move(new_table);
            obj2node_ = std::move(new_obj2node);
            return true;
        }

        res_pair insert(const value_type& obj) {
            lock_t here(mutex_);
            bool inserted{ false };
            iterator iter;
            std::tie(iter, inserted) = insert_obj_(obj);

            size_t tries = 0;
            while (!inserted && tries < 3) {
                rehash();
                std::tie(iter, inserted) = insert_obj_(obj);
            }

            return res_pair(iter, inserted);
        }

        bool erase(iterator iter) {
            lock_t here(mutex_);
            if (iter == end()) {
                return false;
            }

            size_t pos = iter.i();
        }


    private:
        /* @note: it's up to caller to guarantee that obj hasn't been inserted into obj_array.
        */
        res_pair insert_obj_(const value_type& obj) {
            lock_t here(mutex_);
            node_type n;
            n.hash_code = hash_(obj);
            size_t pos = n.hash_code % strategy_.table_size();
            size_t step = 0;
            bool existed{ false };
            for (; step < strategy_.bucket_size(); ++step) {
                if (pos >= table_.size()) {
                    pos -= table_.size();
                }
                if (table_[pos].hash_code == 0)
                    break;
                if (table_[pos].hash_code == n.hash_code && equal_to_(obj, obj_array_[table_[pos].obj_idx])) {
                    existed = true;
                    break;
                }
                ++pos;
            }
            if (existed) {
                return res_pair(iterator(pos, &obj_array_), true);
                //return res_pair(obj_array_.begin() + table_[pos].obj_idx, true);
            }
            if (step == strategy_.bucket_size()) {
                return res_pair(end(), false);
            }

            obj_array_.push_back(obj);
            obj2node_.push_back(pos);
            n.obj_idx = obj_array_.size() - 1;
            table_[pos] = std::move(n);

            return res_pair(iterator(obj_array_.size() - 1, &obj_array_), true);
            //return res_pair(obj_array_.begin() + obj_array_.size() - 1, true);
        }

        bool insert_node_(node_type&& n, node_array_t* pTable, obj2node_t* pObj2node) {
            lock_t here(mutex_);
            size_t pos = n.hash_code % strategy_.table_size();
            size_t step = 0;
            for (; step < strategy_.bucket_size(); ++step) {
                if (pos >= pTable->size()) {
                    pos -= pTable->size();
                }
                if ((*pTable)[pos].hash_code == 0)
                    break;
                if ((*pTable)[pos].hash_code == n.hash_code && (*pTable)[pos].obj_idx == n.obj_idx) {
                    assert(!"obj array must contain unique value!");
                    break;
                }
                ++pos;
            }
            if (step == strategy_.bucket_size()) {
                return false;
            }
            (*pTable)[pos] = std::move(n);
            (*pObj2node)[n.obj_idx] = pos;
            return true;
        }

        size_t hash_(const value_type& obj) const {
            size_t hc = hf_(obj);
            if (hc == 0) {
                hc = 0x0bad;
            }
            return hc;
        }
    };

    //template<typename T_, typename Hash_, typename Equal_ >
    //const Hash_table_<T_, Hash_, Equal_>::iterator Hash_table_<T_, Hash_, Equal_>::npos = Hash_table_<T_, Hash_, Equal_>::iterator(-1, nullptr);

    template<typename T_,
        typename ContainerT_,
        typename Hash_ = mmhash32<T_>,
        typename Equal_ = std::equal_to<T_>,
        typename HT_ = std::unordered_map<T_, size_t, Hash_, Equal_> >
    class Hash_container_
    {
    public:
        typedef T_ Type;
        typedef ContainerT_ container_t;
        typedef HT_ hashtable_t;
        static const size_t npos = size_t(-1);

    private:
        container_t m_container_;
        hashtable_t m_hashtable_;

    public:
        Hash_container_()
        {
        }
        ~Hash_container_()
        {
        }

        size_t end() const {
            return npos;
        }
        size_t find(Type const& v) const
        {
            auto iter = m_hashtable_.find(v);
            if (iter != m_hashtable_.end()) {
                return iter->second;
            }
            return npos;
        }
        bool contains(Type const& v) const
        {
            return m_hashtable_.find(v) != m_hashtable_.end();
        }
        Type const& operator[](size_t index) const {
            return m_container_[index];
        }

        size_t push_back(Type const& v)
        {
            auto idx = find(v);
            if (idx != npos) {
                return idx;
            }
            m_container_.push_back(v);
            idx = m_container_.size() - 1;
            m_hashtable_[v] = idx;
            return idx;
        }
        void erase(Type const& v)
        {
            if (!contains(v)) {
                return;
            }
            m_hashtable_.erase(v);
            return;
        }
        size_t size() const
        {
            return m_container_.size();
        }
        void reserve(size_t sz)
        {
            m_container_.reserve(sz);
        }
        void shrink_to_fit()
        {
            m_container_.shrink_to_fit();
        }
        void clear_container() {
            m_container_.clear();
        }
        void clear_hash() {
            m_hashtable_.clear();
        }
        void clear() {
            clear_hash();
            clear_container();
        }
    };

    template<typename T>
    using HashVector = Hash_container_ < T, std::vector<T> > ;

} // namespace zks;

#endif