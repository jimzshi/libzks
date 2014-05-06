#ifndef _ZKS_BITVECTOR_H
#define _ZKS_BITVECTOR_H

#include "bit_hack.h"
#include "array.h"
#include "u8string.h"

#include <cinttypes>
#include <limits>

namespace zks
{

#ifdef _ZKS64
    class BitVector
    {
        typedef uint64_t word_t;
    private:

        size_t size_;
        LazyArray<word_t> bv_;

        inline size_t word_size_(size_t bitsize) const
        {
            return (bitsize + 63) >> 6;
        }
        inline void reset_tail_()
        {
            int bits_left = 64 - (size_ & 0x3f);
            bv_.last() = (bv_.last() >> bits_left) << bits_left;
        }

    public:
        BitVector() :
                size_(0), bv_()
        {
        }
        BitVector(size_t sz, bool set_ = false)
        {
            resize(sz);
            set_ ? set() : reset();
        }
        void set()
        {
            for (auto& w : bv_) {
                w = ~word_t(0);
            }
            reset_tail_();
        }
        void set(size_t pos)
        {
            if (pos < size_) {
                bv_.at(pos >> 6) |= ((word_t)0x01 << (63 - (pos & 0x3f)));
            }
        }
        void reset()
        {
            for (auto& w : bv_) {
                w = word_t(0);
            }
        }
        void reset(size_t pos)
        {
            if (pos < size_) {
                bv_.at(pos >> 6) &= ~((word_t)0x01 << (63 - (pos & 0x3f)));
            }
        }
        void flip()
        {
            for (auto& w : bv_) {
                w = ~w;
            }
            reset_tail_();
        }
        void flip(size_t pos)
        {
            if (pos < size_) {
                bv_.at(pos >> 6) ^= ((word_t)0x01 << (63 - (pos & 0x3f)));
            }
        }

        void resize(size_t size)
        {
            bv_.resize(word_size_(size));
            if (size < size_) {
                reset_tail_();
            }
            size_ = size;
        }

        size_t size() const {
            return size_;
        }

        bool test(size_t pos) const {
            return (pos < size_) && ((bv_[pos >> 6] >> (63 - (pos & 0x3f))) & (word_t)0x01);
        }
        bool operator[](size_t pos) const {
            return test(pos);
        }
        size_t popcnt() const {
            return popcnt(size_);
        }
        size_t popcnt(size_t size) const {
            size_t res(0), lword(word_size_(size) - 1);
            for (size_t i = 0; i<lword; ++i) {
                res += zks::popcnt(bv_[i]);
            }
            res += zks::popcnt(bv_[lword], (size & 0x1f));
            return res;
        }

        size_t first_bit1() const {
            size_t wsize {word_size_(size_)};
            size_t i = 0;
            for (; i < wsize; ++i) {
                if (bv_[i]) {
                    return (i << 6) + zks::first_bit1(bv_[i]);
                }
            }
            return size_t(-1);
        }

        size_t last_bit1() const {
            size_t wi {word_size_(size_)};
            for (; wi > 0; --wi) {
                size_t w = wi -1;
                if (bv_[w]) {
                    return (w << 6) + zks::last_bit1(bv_[w]);
                }
            }
            return size_t(-1);
        }

        u8string to_u8string() const {
            u8string ret;
            for(size_t i=0; i<bv_.size(); ++i) {
                ret.append(50, "%16.16lX,", bv_[i]);
            }
            return ret;
        }
    };
#else // not _ZKS64
    class BitVector {
        typedef uint32_t word_t;
    private:
        size_t size_;
        LazyArray<word_t> bv_;

        inline size_t word_size_(size_t bitsize) const {
            return (bitsize + 31) >> 5;
        }
        inline void reset_tail_() {
            int bits_left = 32 - (size_ & 0x1f);
            bv_.last() = (bv_.last() >> bits_left) << bits_left;
        }

    public:
        BitVector() : size_(0), bv_() {}
        BitVector(size_t sz, bool set_ = false) {
            resize(sz);
            set_ ? set() : reset();
        }
        void set() {
            for (auto& w : bv_) {
                w = ~word_t(0);
            }
            reset_tail_();
        }
        void set(size_t pos) {
            if (pos<size_) {
                bv_.at(pos >> 5) |= ((word_t)0x01 << (31 - (pos & 0x1f)));
            }
        }
        template<typename ForwardIterator>
        void set(ForwardIterator beg, ForwardIterator end) {
            for (; beg != end; ++beg) {
                set(*beg);
            }
        }
        void reset() {
            for (auto& w : bv_) {
                w = word_t(0);
            }
        }
        void reset(size_t pos) {
            if (pos<size_) {
                bv_.at(pos >> 5) &= ~((word_t)0x01 << (31 - (pos & 0x1f)));
            }
        }
        template<typename _ForwardIterator>
        void reset(_ForwardIterator beg, _ForwardIterator end) {
            for (; beg != end; ++beg) {
                reset(*beg);
            }
        }
        void flip() {
            for (auto& w : bv_) {
                w = ~w;
            }
            reset_tail_();
        }
        void flip(size_t pos) {
            if (pos<size_) {
                bv_.at(pos >> 5) ^= ((word_t)0x01 << (31 - (pos & 0x1f)));
            }
        }

        void resize(size_t size) {
            bv_.resize(word_size_(size));
            if (size<size_) {
                reset_tail_();
            }
            size_ = size;
        }

        size_t size() const {
            return size_;
        }
        bool test(size_t pos) const {
            return (pos < size_) && ((bv_[pos >> 5] >> (31 - (pos & 0x1f))) & (word_t)0x01);
        }
        bool operator[](size_t pos) const {
            return test(pos);
        }
        size_t popcnt() const {
            return popcnt(size_);
        }
        size_t popcnt(size_t size) const {
            size_t res(0), lword(word_size_(size) - 1);
            for (size_t i = 0; i<lword; ++i) {
                res += zks::popcnt(bv_[i]);
            }
            res += zks::popcnt(bv_[lword], (size & 0x1f));
            return res;
        }

        size_t first_bit1() const {
            size_t wsize {word_size_(size_)};
            size_t i = 0;
            for (; i < wsize; ++i) {
                if (bv_[i]) {
                    return (i << 5) + zks::first_bit1(bv_[i]);
                }
            }
            return size_t(-1);
        }

        size_t last_bit1() const {
            size_t wi {word_size_(size_)};
            for (; wi > 0; --wi) {
                size_t w = wi - 1;
                if (bv_[w]) {
                    return (w << 5) + zks::last_bit1(bv_[w]);
                }
            }
            return size_t(-1);
        }

        size_t next_bit1(size_t pos) const {
            size_t wi = pos >> 5;
            size_t ret = zks::first_bit1(bv_[wi], pos & 0x1f);
            if (ret < 32) {
                return (wi << 5) + ret;
            }
            for (++wi; wi < bv_.size(); ++wi) {
                if ( 32 > (ret = zks::first_bit1(bv_[wi]))) {
                    return (wi << 5) + ret;
                }
            }
            return size_t(-1);
        }

        template<typename _Container>
        void get_indices1(_Container& c) const {
            c.resize(popcnt());
            size_t p = first_bit1(), end = last_bit1();
            for (size_t cnt = 0; p <= end; ++cnt, p = next_bit1(p)) {
                c.at(cnt) = p;
            }
            return ;
        }

        u8string to_u8string() const {
            u8string ret;
            for (size_t i = 0; i<bv_.size(); ++i) {
                ret.append(50, "%8.8X,", bv_[i]);
            }
            return ret;
        }
    };
#endif // _ZKS64;

} // namespace zks;

#endif
