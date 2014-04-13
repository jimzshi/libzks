#ifndef ZKS_ARRAY_H_
#define ZKS_ARRAY_H_

#include "bit_hack.h"

#include <type_traits>
#include <algorithm>
#include <memory>

namespace zks {

template<typename T_>
struct Ref_array_traits_ {
	typedef T_* pointer;

	int size;
	int ref;
	pointer data;

	Ref_array_traits_() {
		ref = 1;
		size = 0;
		data = new T_[0];
	}
	Ref_array_traits_(int sz) {
		ref = 1;
		size = sz;
		data = new T_[sz]();
	}
	~Ref_array_traits_() {
		delete[] data;
	}
	Ref_array_traits_* detach() {
		if (ref == 1) {
			return this;
		}
		Ref_array_traits_* p = new Ref_array_traits_(size);
		std::copy(data, data + size, p->data);
		--ref;
		return p;
	}
}; //Ref_array_traits_

template<typename T_, 
    typename StorageT_ = std::conditional_t<std::is_pointer<T_>::value, std::shared_ptr<std::remove_pointer_t<T_>>, T_>,
    typename Impl_ = Ref_array_traits_<StorageT_> 
>
class LazyArray {
public:
    typedef T_ Type;
    typedef StorageT_ StorageType;

protected:
	typedef Impl_ impl_t;
	impl_t* rep;

public:
	LazyArray() :
			rep(new impl_t) {
	}
	LazyArray(int sz) :
			rep(new impl_t(sz)) {
	}
	LazyArray(const LazyArray& vec) {
		vec.rep->ref++;
		rep = vec.rep;
	}
	const LazyArray& operator=(const LazyArray& rh) {
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
	~LazyArray() {
		if (--rep->ref == 0) {
			delete rep;
		}
	}

	int size() const {
		return rep->size;
	}
	int ref() const {
		return rep->ref;
	}
    StorageType const& operator[](int sz) const {
		return *(rep->data + sz);
	}
    StorageType const* begin() const {
		return rep->data;
	}
    StorageType const* end() const {
		return rep->data + rep->size;
	}

    StorageType& at(int sz) {
		rep = rep->detach();
		return *(rep->data + sz);
	}

    int insert(int pos, StorageType const& v) {
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

    void push_back(StorageType const& v) {
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

	int erase(int pos, int n = 1) {
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
	void clear() {
		erase(0, rep->size);
	}
	void reverse() {
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
	void resize(int nsz) {
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
};// LazyArray


template<typename T_, int ChunkSize_, int BlockSize_, typename Vec_ >
class ChunkArray;

template<typename T_,
	class = std::enable_if_t<(!std::is_pointer<T_>::value && !std::is_reference<T_>::value)>
>
struct Chunk_type_traits_ {
	typedef typename std::remove_cv<T_>::type Type_;
	typedef Type_* pointer;
	typedef Type_& reference;
	typedef Type_ const* const_ptr;
	typedef Type_ const& const_ref;
};

template<typename T_, int ChunkSize_, int BlockSize_, typename Vec_>
struct Chunk_size_traits_ {
    typedef ChunkArray<T_, ChunkSize_, BlockSize_, Vec_> ChunkArray_;
    typedef typename Chunk_type_traits_<T_>::Type_ Ty_;
	static const int m_type_bytes_ = sizeof(Ty_);
	static const int m_block_bytes_ = zks::NextPowerOf2<int, BlockSize_>::value;
    static const bool m_using_block_ = (ChunkSize_ == 0) || (ChunkSize_ > (m_block_bytes_ / m_type_bytes_));
    static const int m_chunk_size_ = (ChunkSize_ == 0) || (ChunkSize_ > (m_block_bytes_ / m_type_bytes_)) ?
        (m_block_bytes_ / m_type_bytes_) : ChunkSize_;
	static const int m_chunk_mask_ = m_block_bytes_ - 1;
	static const int m_chunk_shift_ = zks::LogBase2 <int, m_block_bytes_>::value ;

	int m_size_;

    int size_() const { return m_size_; }
    int chunk_index_(int idx) const { return m_using_block_ ? (idx >> m_chunk_shift_) : (idx / m_chunk_size_); }
    int sub_index_(int idx) const { return m_using_block_ ? (idx & m_chunk_mask_) : (idx % m_chunk_size_); }
};

template<typename T_, int ChunkSize_ = 0, int BlockSize_ = 4096,
	typename Vec_ = LazyArray<typename Chunk_type_traits_<T_>::pointer>
>
class ChunkArray : 
    private Chunk_size_traits_<T_, ChunkSize_, BlockSize_, Vec_>,
    private Chunk_type_traits_<T_>
{
public:
    typedef T_ Type;
    typedef typename Vec_::StorageType StorageType;
private:
	typedef Chunk_size_traits_<T_, ChunkSize_, BlockSize_, Vec_> Sizetraits_base_;
    friend Sizetraits_base_;
protected:
	typedef Vec_ Header;
    Header m_header_;
    
public:
    ChunkArray() {}
    ChunkArray(int n) { resize(n); }

    int size() const { return this->size_(); }
    int capacity() const {
        return this->m_chunk_size_ * this->m_header_.size();
    }

    void reserve(int size) { 
        int chunks = this->chunk_index_(size - 1) + 1;
        int old_chunks = m_header_.size();
        if (old_chunks < chunks) {
            m_header_.resize(chunks);
        }
        for (int c = old_chunks; c < chunks; ++c) {
            m_header_.at(c).reset(new Type[this->m_chunk_size_]);
        }
    }
    void resize(int size) {
        reserve(size);
        this->m_size_ = size;
    }
};

} //namespace zks;

#endif
