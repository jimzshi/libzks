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

        int size;
        int ref;
        pointer data;

        Ref_array_traits_()
        {
            ref = 1;
            size = 0;
            data = new T_[0];
        }
        Ref_array_traits_(int sz)
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
        LazyArray(int sz) :
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
        int size() const
        {
            return rep->size;
        }
        int ref() const
        {
            return rep->ref;
        }
        StorageType const& operator[](int sz) const
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

        StorageType& at(int sz)
        {
            rep = rep->detach();
            return *(rep->data + sz);
        }
        Type& last()
        {
            rep = rep->detach();
            return *(rep->data + rep->size - 1);
        }
        Type* data()
        {
            rep = rep->detach();
            return rep->data;
        }

        int insert(int pos, StorageType const& v)
        {
            if (pos > rep->size || pos < 0) {
                return -1;
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
        int erase(int pos, int n = 1)
        {
            if (pos < 0 || pos > (rep->size - 1)) {
                return -1;
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
            int s = rep->size;
            impl_t* p = new impl_t(s);
            for (int i = 0; i < s; ++i) {
                (p->data)[i] = (rep->data)[s - 1 - i];
            }
            if (--rep->ref == 0) {
                delete rep;
            }
            rep = p;
        }
        void resize(int nsz)
        {
            if (nsz < 0) {
                nsz = 0;
            }
            impl_t* p = new impl_t(nsz);
            int copy_size = ((nsz < rep->size) ? nsz : rep->size);
            for (int i = 0; i < copy_size; ++i) {
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

    template<typename T_, int ChunkSize_, int ChunkBytes_, typename Vec_>
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

    template<typename T_, int ChunkSize_, int ChunkBytes_, typename Vec_>
    struct Chunk_size_traits_
    {
        typedef ChunkArray<T_, ChunkSize_, ChunkBytes_, Vec_> ChunkArray_;
        typedef typename Chunk_type_traits_<T_>::Type_ Ty_;
        static const int m_type_bytes_ = sizeof(Ty_);
        static const bool m_using_block_ = (ChunkSize_ == 0);
        static const int m_chunk_bytes_ = m_using_block_ ? zks::NextPowerOf2<int, ChunkBytes_>::value : (m_type_bytes_ * ChunkSize_);
        static const int m_chunk_size_ = m_using_block_ ? (m_chunk_bytes_ / m_type_bytes_) : ChunkSize_;

        int m_size_;

        int size_() const
        {
            return m_size_;
        }
        int chunk_index_(int idx) const
        {
            return idx / m_chunk_size_;
        }
        int sub_index_(int idx) const
        {
            return idx % m_chunk_size_;
        }
        int chunks_cover_(int size) const
        {
            return chunk_index_(size - 1) + 1;
        }
    };

    template<typename T_, int ChunkSize_ = 0, int ChunkBytes_ = 4096,
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
        ChunkArray(int n)
        {
            resize(n);
        }
        ChunkArray(ChunkArray const& rhs) = default;

        int size() const
        {
            return this->size_();
        }
        int chunk_size() const
        {
            return this->m_chunk_size_;
        }
        int chunk_bytes() const
        {
            return this->m_chunk_bytes_;
        }
        int chunks() const
        {
            return m_header_.size();
        }
        int capacity() const
        {
            return chunks() * this->m_chunk_size_;
        }

        void chunk_resize(int new_chunks)
        {
            int old_chunks = chunks();
            if (old_chunks == new_chunks) {
                return;
            }
            m_header_.resize(new_chunks);
            if (new_chunks > old_chunks) {
                for (int c = old_chunks; c < new_chunks; ++c) {
                    //m_header_.at(c).reset(new Type[this->m_chunk_size_], [](Type *p) { delete[] p; }); //TODO: decoupled with shared_ptr interface
                    m_header_.at(c).resize(this->m_chunk_size_);
                }
            }
            else if (new_chunks < this->chunks_cover_(size())) {
                this->m_size_ = capacity();
            }
            return;
        }
        void reserve(int size)
        {
            int new_chunks = this->chunks_cover_(size);
            int old_chunks = chunks();
            if (new_chunks > old_chunks) {
                chunk_resize(new_chunks);
            }
            return;
        }
        void resize(int size)
        {
            reserve(size);
            this->m_size_ = size;
        }
        void shrink_to_fit()
        {
            int new_chunks = this->chunks_cover_(size());
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
        int push_back(Type const& v)
        {
            int sz = size();
            if (sz >= capacity()) {
                append_chunk();
            }
            operator[](size()) = v;
            ++this->m_size_;
            return sz;
        }
        Type const& operator[](int idx) const
        {
            return m_header_[this->chunk_index_(idx)][this->sub_index_(idx)];
        }
        Type& operator[](int idx)
        {
            return m_header_.at(this->chunk_index_(idx)).at(this->sub_index_(idx));
        }
        Type const& at(int idx) const
        {
            if (idx >= size()) {
                throw std::out_of_range("ChunkArray index can't be bigger than its size.");
            }
            return m_header_[this->chunk_index_(idx)][this->sub_index_(idx)];
        }
        Type& at(int idx)
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
        Type* chunk(int c)
        {
            return m_header_[c].data();
        }

    };

} //namespace zks;

#endif
