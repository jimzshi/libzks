#ifndef ZKS_ARRAY_H_
#define ZKS_ARRAY_H_

#include "bit_hack.h"

#include <type_traits>
#include <algorithm>
#include <memory>

namespace zks
{

    template<typename T_>
    struct Ref_array_traits_
    {
        typedef T_* pointer;

        size_t size;
        size_t ref;
        pointer data;

        Ref_array_traits_()
        {
            ref = 1;
            size = 0;
            data = new T_[0];
        }
        Ref_array_traits_(size_t sz)
        {
            ref = 1;
            size = sz;
            data = new T_[sz]();
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
            Ref_array_traits_* p = new Ref_array_traits_(size);
            std::copy(data, data + size, p->data);
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
            vec.rep->ref++;
            rep = vec.rep;
        }
        const LazyArray& operator=(const LazyArray& rh)
        {
            if (rh.rep == rep) {
                return *this;
            }
            if (--rep->ref == 0) {
                delete rep;
            }
            rh.rep->ref++;
            rep = rh.rep;
            return *this;
        }
        ~LazyArray()
        {
            if (--rep->ref == 0) {
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
        size_t ref() const
        {
            return rep->ref;
        }
        StorageType const& operator[](size_t sz) const
        {
            return *(rep->data + sz);
        }
        StorageType const* begin() const
        {
            return rep->data;
        }
        StorageType const* end() const
        {
            return rep->data + rep->size;
        }
        StorageType* begin()
        {
            return rep->data;
        }
        StorageType* end()
        {
            return rep->data + rep->size;
        }

        StorageType& at(size_t sz)
        {
            rep = rep->detach();
            return *(rep->data + sz);
        }
        StorageType& last()
        {
            return back();
        }
        StorageType& front()
        {
            rep = rep->detach();
            return *(rep->data);
        }
        StorageType& back()
        {
            rep = rep->detach();
            return *(rep->data + rep->size - 1);
        }
        StorageType* data()
        {
            rep = rep->detach();
            return rep->data;
        }

        size_t insert(size_t pos, StorageType const& v)
        {
            if (pos > rep->size) {
                return size_t(-1);
            }
            size_t nsz(rep->size + 1);
            impl_t* p = new impl_t(nsz);
            std::copy(rep->data, rep->data + pos, p->data);
            (p->data)[pos] = v;
            std::copy(rep->data + pos, rep->data + rep->size, p->data + pos + 1);
            if (--rep->ref == 0) {
                delete rep;
            }
            rep = p;
            return pos;
        }

        void push_back(StorageType const& v)
        {
            size_t sz = rep->size;
            impl_t* p = new impl_t(sz + 1);
            std::copy(rep->data, rep->data + sz, p->data);
            (p->data)[sz] = v;
            if (--rep->ref == 0) {
                delete rep;
            }
            rep = p;
            return;
        }
        Type& append()
        {
            size_t sz = rep->size;
            impl_t* p = new impl_t(sz + 1);
            std::copy(rep->data, rep->data + sz, p->data);
            if (--rep->ref == 0) {
                delete rep;
            }
            rep = p;
            return rep->data[sz];
        }
        size_t erase(size_t pos, size_t n = 1)
        {
            if (pos > (rep->size - 1)) {
                return size_t(-1);
            }
            if (n > (rep->size - pos)) {
                n = rep->size - pos;
            }
            size_t new_size = rep->size - n;
            impl_t* p = new impl_t(new_size);
            std::copy(rep->data, rep->data + pos, p->data);
            std::copy(rep->data + pos + n, rep->data + rep->size, p->data + pos);
            if (--rep->ref == 0) {
                delete rep;
            }
            rep = p;
            return pos;
        }
        void clear()
        {
            erase(0, rep->size);
        }
        void reverse()
        {
            size_t s = rep->size;
            impl_t* p = new impl_t(s);
            for (size_t i = 0; i < s; ++i) {
                (p->data)[i] = (rep->data)[s - 1 - i];
            }
            if (--rep->ref == 0) {
                delete rep;
            }
            rep = p;
        }
        void resize(size_t nsz)
        {
            if (nsz < 0 || nsz == rep->size) {
                return;
            }
            impl_t* p = new impl_t(nsz);
            size_t copy_size = ((nsz < rep->size) ? nsz : rep->size);
            for (size_t i = 0; i < copy_size; ++i) {
                (p->data)[i] = (rep->data)[i];
            }
            if (--rep->ref == 0) {
                delete rep;
            }
            rep = p;
            return;
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
        static const bool m_using_block_ = (ChunkSize_ == 0);
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
            return chunk_index_(size - 1) + 1;
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
        ChunkArray()
        {
        }
        ChunkArray(size_t n)
        {
            resize(n);
        }
        ChunkArray(ChunkArray const& rhs) = default;

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
