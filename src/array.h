#ifndef ZKS_ARRAY_H_
#define ZKS_ARRAY_H_

#include "bit_hack.h"

#include <type_traits>
#include <algorithm>

namespace zks {

template<typename T_, bool Is_ptr_ = std::is_pointer<T_>::value,
		bool Is_array_ = std::is_array<T_>::value>
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
	Ref_array_traits_(Ref_array_traits_ const& ra) {
		size = ra.size;
		ref = ++ra.ref;
		data = ra.data;
	}
	Ref_array_traits_(Ref_array_traits_ && rv) {
		size = rv.size;
		ref = rv.ref;
		data = rv.data;
		rv.data = nullptr;
	}
	~Ref_array_traits_() {
		if (Is_ptr_) {
			for (int i = 0; i < size; ++i) {
				delete data[i];
				data[i] = nullptr;
			}
		} else if (Is_array_) {
			for (int i = 0; i < size; ++i) {
				delete[] data[i];
			}
		}
		delete[] data;
	}
	Ref_array_traits_ const& operator=(Ref_array_traits_ const& ra) {
		size = ra.size;
		ref = ++ra.ref;
		data = ra.data;
		return *this;
	}
	Ref_array_traits_ const& operator=(Ref_array_traits_&& ra) {
		size = ra.size;
		ref = ra.ref;
		data = ra.data;
		ra.data = nullptr;
		return *this;
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

template<typename T_, typename Impl_ = Ref_array_traits_<T_> >
class LazyArray {
public:
	typedef Impl_ impl_t;

protected:

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
	const T_& operator[](int sz) const {
		return *(rep->data + sz);
	}
	T_ const *begin() const {
		return rep->data;
	}
	T_ const *end() const {
		return rep->data + rep->size;
	}

	T_& at(int sz) {
		rep = rep->detach();
		return *(rep->data + sz);
	}

	int insert(int pos, const T_& v) {
		if (pos > rep->size || pos < 0) {
			return -1;
		}
		size_t nsz(rep->size + 1);
		impl_t* p = new impl_t(nsz);
//		  for(int i=0; i<pos; ++i) {
//			(p->data)[i] = (rep->data)[i];
//		  }
//		  (p->data)[pos] = v;
//		  for(size_t i=pos+1; i<nsz; ++i) {
//			(p->data)[i] = (rep->data)[i-1];
//		  }
		std::copy(rep->data, rep->data + pos, p->data);
		(p->data)[pos] = v;
		std::copy(rep->data + pos, rep->data + rep->size, p->data + pos + 1);
		if (--rep->ref == 0) {
			delete rep;
		}
		rep = p;
		return pos;
	}

	void push_back(const T_& v) {
		size_t sz = rep->size;
		impl_t* p = new impl_t(sz + 1);
//		  for(size_t i=0; i<sz; ++i) {
//			(p->data)[i] = (rep->data)[i];
//		  }
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
//		  for(int i=0; i<pos; ++i) {
//			(p->data)[i] = (rep->data)[i];
//		  }
//		  for(size_t i=pos; i<new_size; ++i) {
//			(p->data)[i] = (rep->data)[n+i];
//		  }
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

template<typename T_, int ChunkSize_, int BlockSize_>
struct Chunk_size_traits_ {
    typedef typename Chunk_type_traits_<T_>::Type_ Ty_;
	static const int m_type_bytes_ = sizeof(Ty_);
	static const int m_block_bytes_ = zks::NextPowerOf2<int, BlockSize_>::value;
	static const int m_chunk_size_ = (ChunkSize_ <= m_block_bytes_ / m_type_bytes_) ?
					ChunkSize_ 	: (m_block_bytes_/m_type_bytes_);
	static const int m_chunk_mask_ = m_block_bytes_ - 1;
	static const int m_chunk_shift_ = zks::LogBase2 <int, m_block_bytes_>::value ;
	int m_size_;
};

template<typename T_, int ChunkSize_, int BlockSize_ = 4096,
	typename Type_traits_ = Chunk_type_traits_<T_>,
	typename Size_traits_ = Chunk_size_traits_<T_, ChunkSize_, BlockSize_>,
	typename Vec_ = LazyArray<typename Type_traits_::pointer>
>
class ChunkArray : protected Size_traits_ {
public:
	typedef typename Type_traits_::Type_ Ty_;
	typedef typename Type_traits_::pointer pointer;
	typedef typename Type_traits_::reference reference;
	typedef typename Type_traits_::const_ptr const_ptr;
	typedef typename Type_traits_::const_ref const_ref;
	typedef Vec_ Header_;
	typedef Size_traits_ Sizetraits_base_;
protected:
	Header_ m_header_;
public:
	int size() const { return this->m_size_; }
};

} //namespace zks;

#endif
