#ifndef ZKS_HASH_H_
#define ZKS_HASH_H_

#include "utility.h"
#include "MurmurHash3.h"
#include "u8string.h"

#include <cstdint>
#include <cstring>
#include <array>
#include <functional>
#include <vector>
#include <unordered_map>

namespace zks {


    template<int NBITS_>
    struct HashResType {};

    template<> struct HashResType < 32 > {
        typedef uint32_t result_type;
        static const int BITS = 32;
    };
    template<> struct HashResType < 64 > {
        typedef uint64_t result_type;
        static const int BITS = 64;
    };
    template<> struct HashResType < 128 > {
        typedef std::array<char, 16> result_type;
        static const int BITS = 128;
    };
    template<> struct HashResType < 256 > {
        typedef std::array<char, 32> result_type;
        static const int BITS = 256;
    };

    template<int NBITS>
    struct HashFuncType {
        typedef typename HashResType<NBITS>::result_type result_type;
        typedef result_type(*hasher)(const void *, int, result_type);
    };

    template<int NBITS>
    struct MurmurHash {
        typedef typename HashFuncType<NBITS>::result_type result_type;
        typedef typename HashFuncType<NBITS>::hasher hasher;
        static const result_type SALT;
        static result_type hash(const void* key, int n, result_type seed);
        static result_type salt(bool fixed = false);
        result_type operator()(const void* key, int n, result_type seed) {
            return MurmurHash::hash(key, n, seed);
        }
    };
    template<> MurmurHash<32>::result_type MurmurHash<32>::salt(bool fixed);
    template<> MurmurHash<32>::result_type MurmurHash<32>::hash(const void* key, int n, result_type seed);


    template<int NBITS_, typename HashTraits = MurmurHash<NBITS_>, typename WordType = uint32_t>
    struct Hashcode_base_ {
    public:
        typedef Hashcode_base_<NBITS_, HashTraits, WordType> Self;
        typedef typename HashTraits::result_type result_type;
        typedef typename HashTraits::hasher hasher;
        typedef WordType word_t;
        typedef uint8_t byte_t;
        static const int BITS = NBITS_,
            WORD_BITS = sizeof(word_t) * 8,
            BYTES = (NBITS_ + 7) / 8,
            WORD_SIZE = (NBITS_ + sizeof(word_t) * 8 - 1) / (sizeof(word_t) * 8);

    public:
        word_t h[WORD_SIZE];

    public:
        Hashcode_base_(bool salt = true) {
            if (salt) {
                std::memcpy((void*)&h, (void*)&HashTraits::SALT, BYTES);
            }
            else {
                std::memset((void*)&h, 0, BYTES);
            }
        }
        ~Hashcode_base_() {}

        template<typename ArgT_>
        Self& operator+=(ArgT_ const& rhs) {
            result_type* seed = (result_type*)&h;
            result_type h2 = HashTraits::hash((void*)&rhs, sizeof(ArgT_), *seed);
            std::memcpy((void*)h, (void*)&h2, BYTES);
            return *this;
        }

        Self& operator+=(const char* rhs) {
            result_type* seed = (result_type*)&h;
            result_type h2 = HashTraits::hash((void*)rhs, std::strlen(rhs), *seed);
            std::memcpy((void*)h, (void*)&h2, BYTES);
            return *this;
        }
        Self& operator+=(u8string const& rhs) {
            return operator+=((const char*)rhs);
        }

        template<int N_, typename H_, typename W_>
        Self& operator+=(Hashcode_base_<N_, H_, W_> const& rhs) {
            result_type* seed = (result_type*)&h;
            result_type h2 = HashTraits::hash((void*)&rhs.h, Hashcode_base_<N_, H_, W_>::BYTES, *seed);
            std::memcpy((void*)h, (void*)&h2, BYTES);
            return *this;
        }
    };

    template<int N_, typename H_, typename W_>
    u8string to_u8string(Hashcode_base_<N_, H_, W_> const& h) {
        typedef typename Hashcode_base_<N_, H_, W_>::word_t word_t;
        static int word_size = sizeof(word_t);
        static int bytes = Hashcode_base_<N_, H_, W_>::BYTES;

        u8string ret;
        uint8_t* p = (uint8_t*)(&h.h);
        for (int i = 0; i < bytes; ++i) {
            ret += zks::as_hex(p[i]);
            if (i != (bytes - 1) && (i + 1) % word_size == 0) {
                ret += '-';
            }
        }
        ret.shrink_to_fit();
        return ret;
    }

    typedef Hashcode_base_<32> HashCode32;

    template<typename T_,
        typename Hash_,
        typename Equal_ = std::equal_to<T_>,
        typename Vec_ = std::vector<T_>,
        typename HT_ = std::unordered_map<T_, size_t, Hash_, Equal_>
    > class HashVector {
    public:
        typedef T_ Type;
        typedef Vec_ vector_t;
        typedef HT_ hashtable_t;
        static const size_t npos = size_t(-1);

    private:
        vector_t m_vec_;
        hashtable_t m_hashtable_;

    public:
        HashVector() {}
        ~HashVector() {}

        size_t find(Type const& v) {
            auto iter = m_hashtable_.find(v);
            if (iter != m_hashtable_.end()) {
                return iter->second;
            }
            return npos;
        }
        bool contains(Type const& v) {
            return m_hashtable_.find(v) != m_hashtable_.end();
        }
        size_t push_back(Type const& v) {
            auto idx = find(v);
            if (idx != npos) {
                return idx;
            }
            m_vec_.push_back(v);
            idx = m_vec_.size() - 1;
            m_hashtable_[v] = idx;
            return idx;
        }
        void erase(Type const& v) {
            if (!contains(v)) {
                return;
            }
            m_hashtable_.erase(v);
            return;
        }
        size_t size() const {
            return m_vec_.size();
        }
        void reserve(size_t sz) {
            m_vec_.reserve(sz);
        }
        void shrink_to_fit() {
            m_vec_.shrink_to_fit();
        }
    };
        
}

#endif
