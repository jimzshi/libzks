#ifndef ZKS_ARRAY_H_
#define ZKS_ARRAY_H_

#include "bit_hack.h"

#include <type_traits>
#include <algorithm>
#include <memory>
#include <cassert>

namespace zks
{

    template<typename T_>
    struct Ref_array_traits_
    {
        typedef T_* pointer;

        pointer data;
        size_t capacity;
        size_t size;
        size_t ref;
        

        Ref_array_traits_() : data(nullptr), capacity(0), size(0), ref(1)
        {
        }
        Ref_array_traits_(size_t sz) : data(new T_[sz]()), capacity(sz), size(0), ref(1)
        {
        }
        ~Ref_array_traits_()
        {
            delete[] data;
        }
        Ref_array_traits_* detach()
        {
            if (ref == 1) {
                return this;
            }
            Ref_array_traits_* p = new Ref_array_traits_(capacity);
            std::copy(data, data + size, p->data);
            p->size = size;
            --ref;
            return p;
        }
    };
    //Ref_array_traits_

    template<typename T_,
            typename StorageT_ = std::conditional_t<std::is_pointer<T_>::value, std::shared_ptr<std::remove_pointer_t<T_>>, T_>,
            typename Impl_ = Ref_array_traits_<StorageT_> >
    class LazyArray
    {
    public:
        typedef T_ Type;
        typedef StorageT_ StorageType;

    protected:
        typedef Impl_ impl_t;
        impl_t* rep;

    public:
        LazyArray() :
                rep(new impl_t)
        {
        }
        LazyArray(size_t sz) :
                rep(new impl_t(sz))
        {
        }
        LazyArray(const LazyArray& vec)
        {
            if(vec.rep)
                ++vec.rep->ref;
            rep = vec.rep;
        }
        LazyArray(LazyArray&& rh)
        {
            rep = rh.rep;
            rh.rep = nullptr;
        }
        const LazyArray& operator=(const LazyArray& rh)
        {
            if (rh.rep == rep) {
                return *this;
            }
            if (rep && --rep->ref == 0) {
                delete rep;
            }
            rh.rep->ref++;
            rep = rh.rep;
            return *this;
        }
        const LazyArray& operator=(LazyArray&& rh)
        {
            if (rep == rh.rep) {
                if(rep)
                    --rep->ref;
                rh.rep = nullptr;
                return *this;
            }
            if(rep && --rep->ref==0) {
                delete rep;
            }
            rep = rh.rep;
            rh.rep = nullptr;
            return *this;
        }
        ~LazyArray()
        {
            if (rep && (--rep->ref == 0) ) {
                delete rep;
            }
        }

        bool empty() const
        {
            return rep->size == 0;
        }
        size_t size() const
        {
            return rep->size;
        }
        size_t capacity() const {
            return rep->capacity;
        }
        size_t ref() const
        {
            return rep->ref;
        }
        StorageType const& operator[](size_t sz) const
        {
            assert(sz < rep->size);
            return *(rep->data + sz);
        }
        StorageType const* cbegin() const
        {
            return rep->data;
        }
        StorageType const* cend() const
        {
            return rep->data + rep->size;
        }
        StorageType const* cdata() const
        {
            return rep->data;
        }
        template<typename Equal_ = std::equal_to<StorageType>>
        size_t find(StorageType const& v, Equal_ eq) const {
            for(size_t i=0; i<rep->size; ++i) {
                if(eq(v, rep->data[i])) {
                    return i;
                }
            }
            return size_t(-1);
        }

        StorageType* begin()
        {
            rep = rep->detach();
            return rep->data;
        }
        StorageType* end()
        {
            rep = rep->detach();
            return rep->data + rep->size;
        }
        StorageType& operator[](size_t sz) {
            assert(sz < rep->size);
            rep = rep->detach();
            return *(rep->data + sz);
        }
        StorageType& at(size_t sz)  //safely grow index;
        {
            if(sz >= rep->size) {
                resize(sz+1);
            }
            rep = rep->detach();
            return *(rep->data + sz);
        }
        StorageType& last()
        {
            rep = rep->detach();
            return *(rep->data + rep->size - 1);
        }
        StorageType& front()
        {
            return *begin();
        }
        StorageType& back()
        {
            return last();
        }
        StorageType* data()
        {
            rep = rep->detach();
            return rep->data;
        }

        void reserve(size_t nsz) {
            if(nsz <= rep->capacity) {
                return;
            }
            size_t new_cap = zks::next_pow2(nsz);
            impl_t* p = new impl_t(new_cap);
            if(rep->ref == 1) {
                std::move(rep->data, rep->data + rep->size, p->data);
            } else {
                std::copy(rep->data, rep->data + rep->size, p->data);
                --rep->ref;
            }
            p->size = rep->size;
            rep = p;
            return;
        }
        void resize(size_t nsz)
        {
            if (nsz == rep->size) {
                return;
            }
            if (nsz > rep->capacity) {
                reserve(nsz);
            } else {
                rep = rep->detach();
            }
            if(nsz < rep->size) {
                std::for_each(rep->data + nsz, rep->data + rep->size, [](StorageType& v) { v.~StorageType();});
            }
            rep->size = nsz;
            return;
        }
        
        void insert_at(size_t pos, StorageType const& v) {
            size_t nsz( std::max(pos, rep->size) + 1);  //for pos > rep->size;
            if(nsz > rep->capacity) {
                reserve(nsz);
            }
            if(rep->ref == 1) {
                for(size_t i=rep->size - 1; i>=pos; --i) {
                    rep->data[i+1] = std::move(rep->data[i]);
                }
            } else {
                impl_t* p = new impl_t(nsz);
                std::copy(rep->data, rep->data + std::min(pos, rep->size), p->data);  // for pos > rep->size;
                std::copy(rep->data + pos, rep->data + rep->size, p->data + pos + 1);
                --rep->ref;
                rep = p;
            }
            (rep->data)[pos] = v;
            rep->size = nsz;
            return;
        }
        size_t insert(size_t pos, StorageType const& v)
        {
            if (pos > rep->size) {
                return size_t(-1);
            }
            insert_at(pos, v);
            return pos;
        }
        size_t erase(size_t pos, size_t n = 1)
        {
            if (pos >= rep->size || n == 0) {
                return size_t(-1);
            }
            if (n > (rep->size - pos)) {
                n = rep->size - pos;
            }
            size_t new_size = rep->size - n;
            if(rep->ref == 1) {
                std::move(rep->data + pos + n, rep->data + rep->size, rep->pos);
                std::for_each(rep->data + new_size, rep->data + rep->size, [](StorageType& v) { v.~StorageType();});
            } else {
                impl_t* p = new impl_t(rep->capacity);
                std::copy(rep->data, rep->data + pos, p->data);
                std::copy(rep->data + pos + n, rep->data + rep->size, p->data + pos);
                --rep->ref;
                rep = p;
            }
            rep->size = new_size;
            return pos;
        }
        void shrink_to_fit() {
            if(rep->size == rep->capacity)
                return;
            impl_t* p = new impl_t(rep->size);
            p->size = rep->size;
            if(rep->ref == 1) {
                std::move(rep->data, rep->data + rep->size, p->data);
            } else {
                std::copy(rep->data, rep->data + rep->size, p->data);
            }
            if(--rep->ref == 0) {
                delete rep;
            }
            rep = p;
        }
        void push_back(StorageType const& v)
        {
            insert_at(rep->size, v);
        }
        StorageType& append()
        {
            insert_at(rep->size, StorageType());
            return last();
        }

        void clear()
        {
            erase(0, rep->size);
        }
        void reverse()
        {
            if(rep->ref == 1) {
                for(size_t i=0, mid = rep->size / 2; i<mid; ++i) {
                    std::swap(rep->data[i], rep->data[rep->size - 1 -i]);
                }
                return;
            }
            
            impl_t* p = new impl_t(rep->capacity);
            for (size_t i = 0; i < rep->size; ++i) {
                (p->data)[i] = (rep->data)[rep->size - 1 - i];
            }
            p->size = rep->size;
            if (--rep->ref == 0) {
                delete rep;
            }
            rep = p;
        }

    };
    // LazyArray

    template<typename T_, size_t ChunkSize_, size_t ChunkBytes_, typename Vec_>
    class ChunkArray;

    template<typename T_, class = std::enable_if_t<(!std::is_pointer<T_>::value && !std::is_reference<T_>::value)> >
    struct Chunk_type_traits_
    {
        typedef typename std::remove_cv<T_>::type Type_;
        typedef Type_* pointer;
        typedef Type_& reference;
        typedef Type_ const* const_ptr;
        typedef Type_ const& const_ref;
    };

    template<typename T_, size_t ChunkSize_, size_t ChunkBytes_, typename Vec_>
    struct Chunk_size_traits_
    {
        typedef ChunkArray<T_, ChunkSize_, ChunkBytes_, Vec_> ChunkArray_;
        typedef typename Chunk_type_traits_<T_>::Type_ Ty_;
        static const size_t m_type_bytes_ = sizeof(Ty_);
        static const bool m_using_block_ = (ChunkSize_ == 0);  // using cache line length (ChunkBytes) as a block;
        static const size_t m_chunk_bytes_ = m_using_block_ ? zks::NextPowerOf2<size_t, ChunkBytes_>::value : (m_type_bytes_ * ChunkSize_);
        static const size_t m_chunk_size_ = m_using_block_ ? (m_chunk_bytes_ / m_type_bytes_) : ChunkSize_;

        size_t m_size_;

        size_t size_() const
        {
            return m_size_;
        }
        size_t chunk_index_(size_t idx) const
        {
            return idx / m_chunk_size_;
        }
        size_t sub_index_(size_t idx) const
        {
            return idx % m_chunk_size_;
        }
        size_t chunks_cover_(size_t size) const
        {
            return size ? chunk_index_(size - 1) + 1 : 1;
        }
    };

    template<typename T_, size_t ChunkSize_ = 0, size_t ChunkBytes_ = 4096,
    //typename Vec_ = LazyArray<typename Chunk_type_traits_<T_>::pointer>
            typename Vec_ = LazyArray<LazyArray<T_> > >
    class ChunkArray: private Chunk_size_traits_<T_, ChunkSize_, ChunkBytes_, Vec_>, private Chunk_type_traits_<T_>
    {
    public:
        typedef T_ Type;
        typedef typename Vec_::StorageType ChunkStorageType;

    protected:
        typedef Vec_ Header;
        Header m_header_;

    public:
        class iterator {
        public:
            typedef std::random_access_iterator_tag iterator_category;
            typedef T_ value_type;
            typedef std::ptrdiff_t difference_type;
            typedef T_& reference;
            typedef T_* pointer;
            typedef iterator Self_;

        private:
            size_t i_;
            ChunkArray* ca_;

        public:
            iterator() : i_(0), ca_(nullptr) {}
            iterator(iterator const& iter) : i_(iter.i_), ca_(iter.ca_) {}
            iterator(size_t i, ChunkArray* ca) : i_(i), ca_(ca) {}
            ~iterator() = default;

            size_t i() const { return i_; }
            reference operator*() { return ca_->at(i_); }
            pointer operator->() { return get(); }
            pointer get() { return &ca_->at(i_); }
            
            Self_& operator++() {
                ++i_;
                return *this;
            }
            Self_& operator++(int) {
                iterator ret(*this);
                ++i_;
                return ret;
            }
            Self_& operator+=(const difference_type& n) {
                i_ += n;
                return *this;
            }

            Self_ operator+(const difference_type& n) const {
                Self_ ret(*this);
                return ret += n;
            }

            Self_& operator--() {
                --i_;
                return *this;
            }
            Self_& operator--(int) {
                iterator ret(*this);
                --i_;
                return ret;
            }
            Self_& operator-=(const difference_type& n) {
                i_ -= n;
                return *this;
            }

            Self_ operator-(const difference_type& n) const {
                Self_ ret(*this);
                return ret -= n;
            }

            int compare(Self_ const& rhs) const
            {
                if (ca_ != rhs.ca_) {
                    throw std::invalid_argument("iterator of different array.");
                }
                if (i_ < rhs.i_) {
                    return -1;
                }
                else if (i_ > rhs.i_) {
                    return 1;
                }
                else {
                    return 0;
                }
            }

            bool operator<(Self_ const& rhs) const {
                return this->compare(rhs) < 0;
            }
            bool operator>(Self_ const& rhs) const {
                return this->compare(rhs) > 0;
            }
            bool operator==(Self_ const& rhs) const {
                return this->compare(rhs) == 0;
            }
            bool operator<=(Self_ const& rhs) const {
                return this->compare(rhs) <= 0;
            }
            bool operator>=(Self_ const& rhs) const {
                return this->compare(rhs) >= 0;
            }
            bool operator!=(Self_ const& rhs) const {
                return this->compare(rhs) != 0;
            }

        }; //class iterator;

        class const_iterator {
        public:
            typedef std::random_access_iterator_tag iterator_category;
            typedef const T_ value_type;
            typedef std::ptrdiff_t difference_type;
            typedef const T_& reference;
            typedef const T_* pointer;
            typedef const_iterator Self_;

        private:
            size_t i_;
            ChunkArray const* ca_;

        public:
            const_iterator() : i_(0), ca_(nullptr) {}
            const_iterator(const_iterator const& iter) : i_(iter.i_), ca_(iter.ca_) {}
            const_iterator(size_t i, ChunkArray* ca) : i_(i), ca_(ca) {}
            ~const_iterator() = default;

            size_t i() const { return i_; }
            reference operator*() { return ca_->at(i_); }
            pointer operator->() { return get(); }
            pointer get() { return &ca_->at(i_); }

            Self_& operator++() {
                ++i_;
                return *this;
            }
            Self_& operator++(int) {
                iterator ret(*this);
                ++i_;
                return ret;
            }
            Self_& operator+=(const difference_type& n) {
                i_ += n;
                return *this;
            }

            Self_ operator+(const difference_type& n) const {
                Self_ ret(*this);
                return ret += n;
            }

            Self_& operator--() {
                --i_;
                return *this;
            }
            Self_& operator--(int) {
                iterator ret(*this);
                --i_;
                return ret;
            }
            Self_& operator-=(const difference_type& n) {
                i_ -= n;
                return *this;
            }

            Self_ operator-(const difference_type& n) const {
                Self_ ret(*this);
                return ret -= n;
            }

            int compare(Self_ const& rhs) const
            {
                if (ca_ != rhs.ca_) {
                    throw std::invalid_argument("iterator of different array.");
                }
                if (i_ < rhs.i_) {
                    return -1;
                }
                else if (i_ > rhs.i_) {
                    return 1;
                }
                else {
                    return 0;
                }
            }

            bool operator<(Self_ const& rhs) const {
                return this->compare(rhs) < 0;
            }
            bool operator>(Self_ const& rhs) const {
                return this->compare(rhs) > 0;
            }
            bool operator==(Self_ const& rhs) const {
                return this->compare(rhs) == 0;
            }
            bool operator<=(Self_ const& rhs) const {
                return this->compare(rhs) <= 0;
            }
            bool operator>=(Self_ const& rhs) const {
                return this->compare(rhs) >= 0;
            }
            bool operator!=(Self_ const& rhs) const {
                return this->compare(rhs) != 0;
            }

        }; //class iterator;

    public:
        ChunkArray()
        {
            resize(0);
        }
        ChunkArray(size_t n)
        {
            resize(n);
        }
        ChunkArray(ChunkArray const& rhs) = default;
        ChunkArray(ChunkArray&& rhs) = default;
        ChunkArray& operator=(const ChunkArray& rhs) = default;
        ~ChunkArray() = default;

        iterator begin() { return iterator(0, this); }
        iterator end() { return iterator(size(), this); }
        const_iterator begin() const { return const_iterator(0, this); }
        const_iterator end() const { return const_iterator(size(), this); }
        const_iterator cbegin() const { return const_iterator(0, this); }
        const_iterator cend() const { return const_iterator(size(), this); }

        size_t size() const
        {
            return this->size_();
        }
        size_t chunk_size() const
        {
            return this->m_chunk_size_;
        }
        size_t chunk_bytes() const
        {
            return this->m_chunk_bytes_;
        }
        size_t chunks() const
        {
            return m_header_.size();
        }
        size_t capacity() const
        {
            return chunks() * this->m_chunk_size_;
        }

        void chunk_resize(size_t new_chunks)
        {
            size_t old_chunks = chunks();
            if (old_chunks == new_chunks) {
                return;
            }
            m_header_.resize(new_chunks);
            if (new_chunks > old_chunks) {
                for (size_t c = old_chunks; c < new_chunks; ++c) {
                    //m_header_.at(c).reset(new Type[this->m_chunk_size_], [](Type *p) { delete[] p; }); //TODO: decoupled with shared_ptr interface
                    m_header_.at(c).resize(this->m_chunk_size_);
                }
            }
            else if (new_chunks < this->chunks_cover_(size())) {
                this->m_size_ = capacity();
            }
            return;
        }
        void reserve(size_t size)
        {
            size_t new_chunks = this->chunks_cover_(size);
            size_t old_chunks = chunks();
            if (new_chunks > old_chunks) {
                chunk_resize(new_chunks);
            }
            return;
        }
        void resize(size_t size)
        {
            reserve(size);
            this->m_size_ = size;
        }
        void shrink_to_fit()
        {
            size_t new_chunks = this->chunks_cover_(size());
            chunk_resize(new_chunks);
            return;
        }
        void append_chunk()
        {
            ChunkStorageType& new_chunk = m_header_.append();
            //new_chunk.reset(new Type[this->m_chunk_size_], [](Type *p) { delete[] p; });
            new_chunk.resize(this->m_chunk_size_);
            return;
        }
        size_t push_back(Type const& v)
        {
            size_t sz = size();
            if (sz >= capacity()) {
                append_chunk();
            }
            operator[](size()) = v;
            ++this->m_size_;
            return sz;
        }
        Type const& operator[](size_t idx) const
        {
            return m_header_[this->chunk_index_(idx)][this->sub_index_(idx)];
        }
        Type& operator[](size_t idx)
        {
            return m_header_.at(this->chunk_index_(idx)).at(this->sub_index_(idx));
        }
        Type const& at(size_t idx) const
        {
            if (idx >= size()) {
                throw std::out_of_range("ChunkArray index can't be bigger than its size.");
            }
            return m_header_[this->chunk_index_(idx)][this->sub_index_(idx)];
        }
        Type& at(size_t idx)
        {
            if (idx >= size()) {
                throw std::out_of_range("ChunkArray index can't be bigger than its size.");
            }
            return m_header_.at(this->chunk_index_(idx)).at(this->sub_index_(idx));
        }
        Type& last()
        {
            return operator[](size() - 1);
        }
        Type* chunk(size_t c)
        {
            return m_header_[c].data();
        }

        
    };

} //namespace zks;

#endif
