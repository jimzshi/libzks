#ifndef _ZKS_UTILITY_H
#define _ZKS_UTILITY_H

#include "u8string.h"

#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <memory>
#include <numeric>
#include <functional>
#include <random>
#include <ctime>

namespace zks
{
    template<typename T>
    class LocalBackup
    {
        T old_;
        T& ref_;
    public:
        LocalBackup(T& v) :
                old_(std::move(v)), ref_(v)
        {
        }
        ~LocalBackup()
        {
            ref_ = std::move(old_);
        }
    };

    template<int R_, typename T_ = size_t>
    class Permutations
    {
    public:
        typedef T_ index_t;
        typedef index_t* ptr_t;
        typedef index_t const* const_ptr;
        typedef Permutations Self;
    private:
        size_t size_;
        ptr_t cardinals_;
        ptr_t indices_;
    public:
        const size_t rank = R_;
        Permutations(index_t const* v)
        {
            cardinals_ = new index_t[rank];
            std::copy(v, v + rank, cardinals_);
            if (!valid_cardinals()) {
                throw std::runtime_error("invalid cardinals.");
            }
            size_ = std::accumulate(cardinals_, cardinals_ + rank, 1, [](index_t const& x, index_t const& y) {return x*y;});
            indices_ = new index_t[rank];
            std::fill_n(indices_, rank, 0);
        }
        Permutations(Self const& rhs) :
                Permutations(rhs.cardinals_)
        {
            std::copy(rhs.indices_, rhs.indices_ + rank, indices_);
        }
        Permutations(Self&& rhs)
        {
            operator=(rhs);
        }
        ~Permutations()
        {
            delete[] cardinals_;
            delete[] indices_;
        }
        Self& operator=(Self const& rhs)
        {
            size_ = rhs.size_;
            std::copy(rhs.cardinals_, rhs.cardinals_ + rank, cardinals_);
            std::copy(rhs.indices_, rhs.indices_ + rank, indices_);
            return *this;
        }
        Self& operator=(Self&& rhs)
        {
            size_ = rhs.size_;
            cardinals_ = rhs.cardinals_;
            rhs.cardinals_ = nullptr;
            indices_ = rhs.indices_;
            rhs.cardinals_ = nullptr;
            return *this;
        }

        Self const& seekp(index_t const* v)
        {
            std::copy(v, v + rank, indices_);
            if (!valid_indices()) {
                throw std::runtime_error("invalid starting indices.");
            }
            return *this;
        }
        Self const& seekp(size_t p)
        {
            for (size_t i = 0; i < rank; ++i) {
                indices_[i] = p % cardinals_[i];
                p /= cardinals_[i];
            }
            return *this;
        }
        void next()
        {
            for (size_t i = 0; i < rank; ++i) {
                if (indices_[i] < cardinals_[i] - 1) {
                    ++indices_[i];
                    return;
                }
                indices_[i] = 0;
            }
        }
        void prev()
        {
            for (size_t i = 0; i < rank; ++i) {
                if (indices_[i] > 0) {
                    --indices_[i];
                    return;
                }
                indices_[i] = cardinals_[i] - 1;
            }
        }

        size_t size() const
        {
            return size_;
        }
        bool valid_cardinals() const
        {
            return std::none_of(cardinals_, cardinals_ + rank, [](index_t const& c) {return c < 1;});
        }
        bool valid_indices() const
        {
            for (size_t i = 0; i < rank; ++i) {
                if (indices_[i] >= cardinals_[i] || indices_[i] < 0)
                    return false;
            }
            return true;
        }
        index_t index(size_t i) const
        {
            if (i < 0 || i >= rank) {
                throw std::runtime_error("out of rank.");
            }
            return indices_[i];
        }
        index_t cardinal(size_t i) const
        {
            if (i < 0 || i >= rank) {
                throw std::runtime_error("out of rank.");
            }
            return cardinals_[i];
        }
        const_ptr indices() const
        {
            return indices_;
        }
        size_t tellp() const
        {
            size_t pos(0);
            size_t acc(1);
            for (size_t i = 0; i < rank; ++i) {
                pos += indices_[i] * acc;
                acc *= cardinals_[i];
            }
            return pos;
        }
    };

    std::vector<zks::u8string> get_mac_address();

    u8string as_hex(uint8_t c);
    u8string as_hex(uint8_t* p, size_t len);
} // namespace zks;

namespace std
{

    template<typename Tp_>
    std::string to_string(const std::vector<Tp_>& vec)
    {
        std::string ret;
        ret += "[";
        auto begin = vec.cbegin();
        auto end = vec.cend();
        for (auto iter = begin; iter != end; ++iter) {
            if (iter == begin) {
                ret += std::to_string(*iter);
            }
            else {
                ret += "," + std::to_string(*iter);
            }
        }
        ret += "]";
        return ret;
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
    {
        os << to_string(vec);
        return os;
    }

    template<typename Tp_>
    std::string to_string(const std::set<Tp_>& s)
    {
        std::string ret;
        ret += "[";
        auto begin = s.cbegin();
        auto end = s.cend();
        for (auto iter = begin; iter != end; ++iter) {
            if (iter == begin) {
                ret += std::to_string(*iter);
            }
            else {
                ret += "," + std::to_string(*iter);
            }
        }
        ret += "]";
        return ret;
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const std::set<T>& ms_set)
    {
        os << to_string(ms_set);
        return os;
    }

    template<typename T1_, typename T2_>
    std::string to_string(std::ostream& os, const std::pair<T1_, T2_>& p)
    {
        return std::string(std::to_string(p.first) + ":" + std::to_string(p.second));
    }

    template<typename T1_, typename T2_>
    std::ostream& operator<<(std::ostream& os, const std::pair<T1_, T2_>& p)
    {
        os << to_string(p);
        return os;
    }

    template<typename T1, typename T2>
    std::string to_string(const std::map<T1, T2>& ms_map)
    {
        std::string ret;
        ret += "{";
        for (const auto& p : ms_map)
            ret += to_string(p) + ",";
        ret += "}";
        return ret;
    }

    template<typename T1, typename T2>
    std::ostream& operator<<(std::ostream& os, const std::map<T1, T2>& ms_map)
    {
        os << to_string(ms_map);
        return os;
    }

} /* std */

#endif // !_ZKS_UTILITY_H
