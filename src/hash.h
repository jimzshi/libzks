#ifndef ZKS_HASH_H_
#define ZKS_HASH_H_

#include "utility.h"
#include "MurmurHash3.h"
#include "u8string.h"
#include "array.h"

#include <cstdint>
#include <cstring>
#include <array>
#include <functional>
#include <vector>
#include <unordered_map>

namespace zks
{

    template<int NBITS_>
    struct HashResType
    {
    };

    template<> struct HashResType<32>
    {
        typedef uint32_t result_type;
        static const int BITS = 32;
    };
    template<> struct HashResType<64>
    {
        typedef uint64_t result_type;
        static const int BITS = 64;
    };
    template<> struct HashResType<128>
    {
        typedef std::array<uint32_t, 4> result_type;
        static const int BITS = 128;
    };
    template<> struct HashResType<256>
    {
        typedef std::array<uint32_t, 8> result_type;
        static const int BITS = 256;
    };

    template<int NBITS>
    struct HashFuncType
    {
        typedef typename HashResType<NBITS>::result_type result_type;
        typedef result_type (*hasher_type)(const void *, size_t, result_type);
    };

    template<int NBITS>
    struct MurmurHash
    {
        typedef typename HashFuncType<NBITS>::result_type result_type;
        typedef typename HashFuncType<NBITS>::hasher_type hasher_type;
        static const result_type SALT;
        static result_type hash(const void* key, size_t n, result_type seed);
        static result_type salt(bool fixed = false);
        result_type operator()(const void* key, size_t n, result_type seed)
        {
            return MurmurHash::hash(key, n, seed);
        }
    };
    template<> MurmurHash<32>::result_type MurmurHash<32>::salt(bool fixed);
    template<> MurmurHash<32>::result_type MurmurHash<32>::hash(const void* key, size_t n, result_type seed);

    template<> MurmurHash<64>::result_type MurmurHash<64>::salt(bool fixed);
    template<> MurmurHash<64>::result_type MurmurHash<64>::hash(const void* key, size_t n, result_type seed);

    template<> MurmurHash<128>::result_type MurmurHash<128>::salt(bool fixed);
    template<> MurmurHash<128>::result_type MurmurHash<128>::hash(const void* key, size_t n, result_type seed);

    template<int NBITS_, typename HashTraits = MurmurHash<NBITS_>, typename WordType = uint32_t>
    struct Hashcode_base_
    {
        typedef Hashcode_base_<NBITS_, HashTraits, WordType> Self;
        typedef typename HashTraits::result_type result_type;
        typedef typename HashTraits::hasher_type hasher_type;
        typedef WordType word_t;
        typedef uint8_t byte_t;
        static const size_t BITS = NBITS_,
                WORD_BITS = sizeof(word_t) * 8,
                BYTES = (NBITS_ + 7) / 8,
                WORD_SIZE = (NBITS_ + sizeof(word_t) * 8 - 1) / (sizeof(word_t) * 8);

    public:
        word_t h[WORD_SIZE];
        hasher_type hasher = HashTraits::hash;

    public:
        Hashcode_base_(bool use_salt = true)
        {
            result_type s = HashTraits::salt(!use_salt);
            std::memcpy((void*)&h, (void*)&s, BYTES);
        }
        Hashcode_base_(const Hashcode_base_& rh) {
            for (size_t i = 0; i < WORD_SIZE; ++i) {
                h[i] = rh.h[i];
            }
        }
        ~Hashcode_base_()
        {
        }

        const Hashcode_base_& operator=(Hashcode_base_ const& rh) {
            if (h != rh.h) {
                for (size_t i = 0; i < WORD_SIZE; ++i) {
                    h[i] = rh.h[i];
                }
            }
            return *this;
        }


        template<typename ArgT_>
        Self& operator+=(ArgT_ const& rhs)
        {
            result_type* seed = (result_type*) &h;
            result_type h2 = HashTraits::hash((void*) &rhs, sizeof(ArgT_), *seed);
            std::memcpy((void*) h, (void*) &h2, BYTES);
            return *this;
        }

        Self& operator+=(const char* rhs)
        {
            result_type* seed = (result_type*) &h;
            result_type h2 = HashTraits::hash((void*) rhs, std::strlen(rhs), *seed);
            std::memcpy((void*) h, (void*) &h2, BYTES);
            return *this;
        }
        Self& operator+=(u8string const& rhs)
        {
            return operator+=(rhs.c_str());
        }

        template<int N_, typename H_, typename W_>
        Self& operator+=(Hashcode_base_<N_, H_, W_> const& rhs)
        {
            result_type* seed = (result_type*) &h;
            result_type h2 = HashTraits::hash((void*) &rhs.h, Hashcode_base_<N_, H_, W_>::BYTES, *seed);
            std::memcpy((void*) h, (void*) &h2, BYTES);
            return *this;
        }
    };

    template<int N_, typename H_, typename W_>
    u8string to_u8string(Hashcode_base_<N_, H_, W_> const& h)
    {
        typedef typename Hashcode_base_<N_, H_, W_>::word_t word_t;
        static int word_size = sizeof(word_t);
        static int bytes = Hashcode_base_<N_, H_, W_>::BYTES;

        u8string ret;
        uint8_t* p = (uint8_t*) (&h.h);
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
    typedef Hashcode_base_<128> HashCode128;

    template<typename T, int NBITS = 32, bool USE_SALT = false, typename WORD_TYPE = uint32_t>
    struct MMHash_base_
    {
        typedef Hashcode_base_<NBITS, MurmurHash<NBITS>, WORD_TYPE> hashcode_t;
        size_t operator()(T const& v) const
        {
            hashcode_t hc{ USE_SALT };
            hc += v;
            return hc.h[0];
        }
    };

    template<typename T>
    using mmhash32 = MMHash_base_<T, 32>;

}

#endif
