#ifndef _ZKS_U8STRING_H
#define _ZKS_U8STRING_H

#if defined (_WIN32) && defined(__AFX_H__)
#define _HAS_WIN32_CSTRING
#endif

#if defined(_WIN32)
#define _HAS_CHAR_T_SUPPORT
#endif

#if defined(_WIN32) || defined(__clang__)
#define _HAS_CODECVT
#endif

#include "iterator.h"

#include <string>
#include <codecvt>
#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <vector>
#include <type_traits>
#include <locale>



//#undef _ZKS_U8STRING_NOVALIDATION
#define _ZKS_U8STRING_INDEX 1



namespace zks {

namespace unicode {

    template<class Facet>
    struct deletable_facet : Facet {
        using Facet::Facet; // inherit constructors
        ~deletable_facet() {}
    };
    typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> Mbwc_codecvt;
    //typedef deletable_facet<std::codecvt_byname<wchar_t, char, std::mbstate_t>> Mbwc_codecvt;
    typedef std::wstring_convert<Mbwc_codecvt> Mbwc_cvt;

//helpers
#ifdef _HAS_CHAR_T_SUPPORT
    extern std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> u16u8_cvt;
    extern std::wstring_convert<std::codecvt_utf8<char32_t, 0x7fffffff>, char32_t> u32u8_cvt;
#endif
	extern std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcu8_cvt;

	char16_t to_char16(const char* s);
	bool validate_u8char(const char* s, size_t* pLen = nullptr);
	bool validate_range(const char* i1, const char* i2, size_t* pLen = nullptr);
	/*
	 * @para pLen [out] how many valid u8 bytes in the string.
	*/
	inline bool validate(const char* s, size_t* pLen = nullptr) { return validate_range(s, s + std::char_traits<char>::length(s), pLen); }
	template<typename T_>
	bool invalid_codepoint(T_ cp) { return cp >= 0xd800 && cp <= 0xdfff; }
}


	template<typename T_>
	class u8_iterator_ {
	public:
		typedef std::forward_iterator_tag	iterator_category;
		typedef T_  						value_type;
		typedef std::ptrdiff_t 				difference_type;
		typedef T_& 						reference;
		typedef T_*   						pointer;
		typedef u8_iterator_<T_>			Self_;
	private:
		pointer ptr_;
		size_t len_;

	public:
		u8_iterator_() : ptr_(nullptr), len_(0) {}
		u8_iterator_(pointer p) : ptr_(p)	{
			if (!unicode::validate_u8char(ptr_, &len_)) {
#ifndef _ZKS_U8STRING_NOVALIDATION
				throw std::runtime_error("invalid utf8 char iterator.");
#endif
			}
		}
		u8_iterator_(u8_iterator_ const& p) : ptr_(p.ptr_), len_(p.len_) {}
		u8_iterator_(u8_iterator_ && p) : ptr_(std::move(p.ptr_)), len_(std::move(p.len_)) {}
		~u8_iterator_() {}

		// Forward iterator requirements
		reference operator*() const	{
			return *ptr_;
		}

		pointer	operator->() const {
			return ptr_;
		}

		pointer get() const {
			return ptr_;
		}

		Self_& operator++() {
			ptr_ += len_;
			if (!unicode::validate_u8char(ptr_, &len_)) {
#ifndef _ZKS_U8STRING_NOVALIDATION
				throw std::runtime_error("invalid utf8 char iterator.");
#endif
			}
			return *this;
		}

		Self_ operator++(int) {
			Self_ ret(*this);
			return ++ret;
		}

		Self_& operator+=(const difference_type& n) {
			for (difference_type i = 0; i < n; ++i) operator++();
			return *this;
		}

		Self_ operator+(const difference_type& n) const {
			Self_ ret(ptr_);
			return ret += n;
		}

		size_t len() const {
			return len_;
		}
		pointer begin() const { return ptr_; }
		pointer end() const { return ptr_ + len_; }

		int compare(Self_ const& rhs) const {
			if (ptr_ < rhs.ptr_) {
				return -1;
			}
			else if (ptr_ > rhs.ptr_) {
				return 1;
			}
			else {
				return 0;
			}
		}
	};

	template<typename charT>
	bool operator< (u8_iterator_<charT> const& lhs, u8_iterator_<charT> const& rhs) {
		return lhs.compare(rhs) < 0;
	}
	template<typename charT>
	bool operator> (u8_iterator_<charT> const& lhs, u8_iterator_<charT> const& rhs) {
		return lhs.compare(rhs) > 0;
	}
	template<typename charT>
	bool operator== (u8_iterator_<charT> const& lhs, u8_iterator_<charT> const& rhs) {
		return lhs.compare(rhs) == 0;
	}
	template<typename charT>
	bool operator!= (u8_iterator_<charT> const& lhs, u8_iterator_<charT> const& rhs) {
		return lhs.compare(rhs) != 0;
	}
	template<typename charT>
	bool operator<= (u8_iterator_<charT> const& lhs, u8_iterator_<charT> const& rhs) {
		return lhs.compare(rhs) <= 0;
	}
	template<typename charT>
	bool operator>= (u8_iterator_<charT> const& lhs, u8_iterator_<charT> const& rhs) {
		return lhs.compare(rhs) <= 0;
	}

	class u8string
	{
	public:
		//typedefs
		typedef std::char_traits<char>			traits_type;

		typedef std::string::pointer			pointer;
		typedef std::string::const_pointer		const_pointer;
		typedef std::string::reference			reference;
		typedef std::string::const_reference	const_reference;
		typedef std::string::difference_type	difference_type;
		typedef std::string::size_type			size_type;
		typedef std::string::value_type			value_type;

		typedef std::string::iterator							iterator;
		typedef std::string::const_iterator						const_iterator;
		typedef std::string::reverse_iterator					reverse_iterator;
		typedef std::string::const_reverse_iterator				const_reverse_iterator;

		typedef u8_iterator_<char>										u8_iterator;
		typedef u8_iterator_<const char>								const_u8_iterator;

		typedef u8string	Self_;

		static const size_type npos = -1;

	private:
#if _ZKS_U8STRING_INDEX
		mutable bool changed_;
		mutable bool is_valid_;
		mutable std::vector<size_type> index_;
#endif
		std::string str_;

		void on_change_() const { 
#if _ZKS_U8STRING_INDEX
			changed_ = true; is_valid_ = false; index_.clear(); 
#endif
		}
		int update_() const;
	public:
		/*****************
		 * 21.4.2, construct/copy/destroy:
		 *****************/
		u8string() : str_() { on_change_(); }
		u8string(const char* s) : u8string(s, 0, -1) {}
		u8string(const char* s, size_type n) : u8string(s, 0, n) {}
		u8string(const char* s, size_type pos, size_type n) { assign(s, pos, n); }
		template<typename charT>
		explicit u8string(const charT* s) { assign(s, 0, npos); }
		template<typename charT>
		u8string(const charT* s, size_type n) { assign(s, 0, n); }
		template<typename charT>
		u8string(const charT* s, size_type pos, size_type n) { assign(s, pos, n); }

		template<typename charT>
		u8string(size_t size, charT c) { assign(size, c); }
		u8string(size_t size, const char* str) { assign(size, str); }
		u8string(u8string && rh) { assign(std::move(rh)); }
		u8string(u8string const& s) { assign(s); }
		u8string(const u8string& str, size_type pos, size_type n = npos) : u8string(str.substr(pos, n)) {}
		template<class InputIterator>
		u8string(InputIterator begin, InputIterator end) {
			//size_type dist = std::distance(begin, end);
			//reserve(dist); 
			//since we don't know what's the length of *i, so we couldn't know accurate size to reserve. (char, char16, char32, const char*);
			for (auto i = begin; i != end; ++i) {
				append(*i);
			}
		}
		u8string(std::string const& s) { assign(s.data()); }
		//u8string(std::initializer_list<char> ilist) : u8string(ilist.begin(), ilist.end()) {}
#ifdef _HAS_CHAR_T_SUPPORT
		u8string(std::initializer_list<char32_t> ilist) : u8string(ilist.begin(), ilist.end()) {}
		explicit u8string(std::u16string const& s) { operator=(s); }
		explicit u8string(std::u32string const& s) { operator=(s); }
#endif
		explicit u8string(std::wstring const& s) { operator=(s); }
		template<typename charT>
		explicit u8string(u8_iterator_<charT> const& i) { operator=(i); }
		~u8string() = default;

		u8string& operator=(char c) { str_.resize(1); str_[0] = c; on_change_(); return *this; }
#ifdef _HAS_CHAR_T_SUPPORT
		u8string& operator=(char16_t c16) { str_ = unicode::u16u8_cvt.to_bytes(c16); on_change_(); return *this; }
		u8string& operator=(char32_t c32) { str_ = unicode::u32u8_cvt.to_bytes(c32); on_change_(); return *this; }
#endif
		u8string& operator=(wchar_t wc) { str_ = unicode::wcu8_cvt.to_bytes(wc); on_change_(); return *this; }
		u8string& operator=(const char* cstr) { return assign(cstr); }
		u8string& operator=(u8string const& u8s) {
			if (&u8s != this) {
				assign(u8s);
			}
			return *this;
		}
		u8string& operator=(u8string && u8s) { return assign(std::move(u8s)); }
		u8string& operator=(std::string const& s) { return assign(s.c_str(), 0, s.size()); }
#ifdef _HAS_CHAR_T_SUPPORT
		u8string& operator=(std::u16string const& u16s) { str_ = unicode::u16u8_cvt.to_bytes(u16s); on_change_(); return *this; }
		u8string& operator=(std::u32string const& u32s) { str_ = unicode::u32u8_cvt.to_bytes(u32s); on_change_(); return *this; }
		u8string& operator=(std::initializer_list<char32_t> ilist) { operator=(u8string(ilist)); on_change_(); return *this; }
#endif
		u8string& operator=(std::wstring const& ws) { str_ = unicode::wcu8_cvt.to_bytes(ws); on_change_(); return *this; }
		//u8string& operator=(std::initializer_list<char> ilist) { operator=(u8string(ilist)); on_change_(); return *this; }
		template<typename charT>
		u8string& operator=(u8_iterator_<charT> const& i) { return assign(i.get(), 0, i.len()); }

		/***************
		 * 21.4.3, iterators:
		 ***************/
		iterator begin() { on_change_();  return str_.begin(); }
		const_iterator begin() const { return str_.begin(); }
		const_iterator cbegin() const { return str_.cbegin(); }
		iterator end() { on_change_(); return str_.end(); }
		const_iterator end() const { return str_.end(); }
		const_iterator cend() const { return str_.cend(); }
		reverse_iterator rbegin() { on_change_();  return str_.rbegin(); }
		const_reverse_iterator rbegin() const { return str_.rbegin(); }
		const_reverse_iterator crbegin() const { return str_.crbegin(); }
		reverse_iterator rend() { on_change_(); return str_.rend(); }
		const_reverse_iterator rend() const { return str_.rend(); }
		const_reverse_iterator crend() const { return str_.crend(); }

		u8_iterator u8_begin() { on_change_(); return u8_iterator(&str_[0]); }
		const_u8_iterator u8_begin() const { return const_u8_iterator(&str_[0]); }
		const_u8_iterator u8_cbegin() const { return const_u8_iterator(&str_[0]); }
		u8_iterator u8_end() { on_change_(); return u8_iterator(&str_[0] + str_.size()); }
		const_u8_iterator u8_end() const { return const_u8_iterator(&str_[0] + str_.size()); }
		const_u8_iterator u8_cend() const { return const_u8_iterator(&str_[0] + str_.size()); }

		/******************
		 * 21.4.4, capacity:
		 ******************/
		void clear() { str_.clear(); }
		bool empty() const { return str_.empty(); }
		size_type size() const { return str_.size(); }
		size_type length() const { return str_.length(); }
		size_type max_size() const { return str_.max_size(); }
		void resize(size_type count) { on_change_(); str_.resize(count); return; }
		void resize(size_type count, char c) { on_change_(); str_.resize(count, c); return; }
		void reserve(size_type c) { str_.reserve(c); }
		size_type capacity() const { return str_.capacity(); }
		void shrink_to_fit() { str_.shrink_to_fit(); }


		/**************
		* 21.4.5, element access:
		**************/
		reference at(size_type n) { on_change_(); return str_.at(n); }
		const_reference at(size_type n) const { return str_.at(n); }
		reference operator[](size_type n) { on_change_(); return str_[n]; }
		const_reference operator[](size_type n) const { return str_[n]; }
		reference front() { on_change_(); return str_.front(); }
		const_reference front() const { return str_.front(); }
		reference back() { on_change_(); return str_.back(); }
		const_reference back() const { return str_.back(); }


		/*******************
		 * 21.4.6, modifiers:
		 *******************/
		// 21.4.6.1 operator+=
		u8string& operator+=(const u8string& str) { return append(str); }
		u8string& operator+=(const char* s) { return append(s); }
		u8string& operator+=(char c) { return append(c); }
#ifdef _HAS_CHAR_T_SUPPORT
		u8string& operator+=(char16_t c16) { return append(c16); }
		u8string& operator+=(char32_t c32) { return append(c32); }
		u8string& operator+=(std::initializer_list<char32_t> ilist) { return append(ilist); }
#endif
		u8string& operator+=(wchar_t wc) { return append(wc); }

		// 21.4.6.2 append()
		u8string& append(const u8string& str) { return append(str.data(), str.size()); }
		u8string& append(const u8string& str, size_type pos, size_type n) {
			if (pos > str.size()) {
				throw std::out_of_range("pos is beyond str.size()");
			}
			return append(str.data() + pos, n);
		}
		u8string& append(const char* s, size_type n); //cpp
		u8string& append(const char* s) { return append(s, npos); }
		u8string& append(char* s, size_type n){ return append((const char*)s, n); }
		u8string& append(char* s) { return append(s, npos); }

		u8string& append(size_type n, char c) { return append(u8string(n, c)); }
#ifdef _HAS_CHAR_T_SUPPORT
		u8string& append(size_type n, char16_t c16);	//cpp
		u8string& append(size_type n, char32_t c32);	//cpp
		u8string& append(std::initializer_list<char32_t> ilist) { return append(ilist.begin(), ilist.end()); }
#endif
		u8string& append(size_type n, wchar_t wc);	//cpp
		template<typename charT>
		u8string& append(charT c) { return append(1, c); }

		template<class InputIterator>
		u8string& append(InputIterator first, InputIterator last) {
			//size_type dist = std::distance(first, last);
			//reserve(size() + dist);
			for (auto i = first; i != last; ++i) {
				append(*i);
			}
			return *this;
		}

		template<typename charT>
		void push_back(charT c) { append(1, c); }

		// 21.4.6.3 assign()
		u8string& assign(const char* s, size_type pos, size_type cnt); //cpp
		template<typename charT> //wchar_t, char16_t, char32_t;
		u8string& assign(const charT* s, size_type pos, size_type cnt) {
			if (!s || cnt == 0) {
				return *this;
			}
			size_type sz = std::char_traits<charT>::length(s);
			sz = std::min(sz, pos + cnt);
			cnt = std::min(sz - pos, cnt);

			str_.clear();
			for (size_type offset = 0; offset < cnt; ++offset)
				append(s[pos + offset]);

			on_change_();
			return *this;
		}
		template<typename charT> //wchar_t, char16_t, char32_t;
		u8string& assign(const charT* s, size_type sz) { return assign(s, 0, sz); }
		template<typename charT> //wchar_t, char16_t, char32_t;
		u8string& assign(const charT* s) { return assign(s, 0, npos); }

		u8string& assign(size_type s, const char* str);  //cpp
		u8string& assign(size_type s, char c) { str_.assign(s, c); on_change_(); return *this; }
#ifdef _HAS_CHAR_T_SUPPORT
		u8string& assign(size_type s, char16_t c16) { std::string u8 = unicode::u16u8_cvt.to_bytes(c16); return assign(s, u8.data()); }
		u8string& assign(size_type s, char32_t c32) { std::string u8 = unicode::u32u8_cvt.to_bytes(c32); return assign(s, u8.data()); }
		u8string& assign(std::initializer_list<char32_t> ilist) { return assign(ilist.begin(), ilist.end()); }
#endif
		u8string& assign(size_type s, wchar_t wc) { std::string u8 = unicode::wcu8_cvt.to_bytes(wc); return assign(s, u8.data()); }
		u8string& assign(size_type s, u8string const& u8s) { return assign(s, u8s.data()); }

		u8string& assign(u8string const& u8s) { str_.assign(u8s.str_); on_change_(); return *this; }
		u8string& assign(u8string && u8s) { str_.assign(std::move(u8s.str_)); on_change_(); return *this; }
		u8string& assign(u8string const& u8s, size_t pos, size_t cnt) { assign(u8s.substr(pos, cnt)); on_change_(); return *this; }
		template<class InputIterator>
		u8string& assign(InputIterator first, InputIterator last) {
			return assign(u8string(first, last));
		}

		//24.4.6.4 insert()
		u8string& insert(size_type index, char c) { on_change_(); str_.insert(index, 1, c); return *this; }
		u8string& insert(size_type index, size_type count, char c) { on_change_(); str_.insert(index, count, c); return *this; }
		u8string& insert(size_type index, size_type count, const char* str, size_type str_cnt);	//cpp
		u8string& insert(size_type index, const char* s, size_type cnt = size_type(-1)) { return insert(index, 1, s, cnt); }
		u8string& insert(size_type index, u8string const& str) { on_change_(); str_.insert(index, str.str_); return *this; }
		u8string& insert(size_type index, u8string const& str, size_type index_str, size_type count) { return insert(index, str.substr(index_str, count)); }
		iterator insert(const_iterator pos, char c) { return insert(pos, 1, c); }
		iterator insert(const_iterator pos, size_type count, char ch) {
			size_type offset = std::distance(cbegin(), pos);
			insert(offset, count, ch);
			return (begin() + offset);
		}
		template< class InputIt >
		iterator insert(const_iterator pos, InputIt first, InputIt last) {
			size_type offset = std::distance(cbegin(), pos);
			u8string tmp{ first, last };
			insert(offset, tmp);
			return (begin() + offset);
		}
#ifdef _HAS_CHAR_T_SUPPORT
		iterator insert(const_iterator pos, std::initializer_list<char32_t> ilist) { return insert(pos, ilist.begin(), ilist.end()); }
#endif
		/*iterator insert(const_iterator pos, std::initializer_list<char> ilist);*/

		//21.4.6.5 erase()
		u8string& erase(size_type pos = 0, size_type n = npos) { str_.erase(pos, n); on_change_(); return *this; }
		iterator erase(const_iterator p) { on_change_(); return str_.erase(p); }
		iterator erase(const_iterator first, const_iterator last) { on_change_(); return str_.erase(first, last); }
		void pop_back() { on_change_(); str_.pop_back(); }

		//21.4.6.6 replace()
		u8string& replace(size_type pos1, size_type n1, const u8string& str) { str_.replace(pos1, n1, str.str_); on_change_(); return *this; }
		u8string& replace(size_type pos1, size_type n1, const u8string& str, size_type pos2, size_type n2) { str_.replace(pos1, n1, str.str_, pos2, n2); on_change_(); return *this; }
		u8string& replace(size_type pos, size_type n1, const char* s, size_type n2);//cpp
		u8string& replace(size_type pos, size_type n1, const char* s) { return replace(pos, n1, s, npos); }
		u8string& replace(size_type pos, size_type n1, size_type n2, char c) { str_.replace(pos, n1, n2, c); on_change_(); return *this; }
		u8string& replace(const_iterator i1, const_iterator i2, const u8string& str) { str_.replace(i1, i2, str.str_); on_change_(); return *this; }
		u8string& replace(const_iterator i1, const_iterator i2, const char* s, size_type n) { return replace(i1 - cbegin(), i2 - i1, s, n); }
		u8string& replace(const_iterator i1, const_iterator i2, const char* s) { return replace(i1, i2, s, npos); }
		u8string& replace(const_iterator i1, const_iterator i2, size_type n, char c) { str_.replace(i1, i2, n, c); on_change_(); return *this; }
		template<class InputIterator>
		u8string& replace(const_iterator i1, const_iterator i2, InputIterator j1, InputIterator j2) { return replace(i1, i2, u8string(j1, j2));	}
#ifdef _HAS_CHAR_T_SUPPORT
		u8string& replace(const_iterator i1, const_iterator i2, std::initializer_list<char32_t> ilist) { return replace(i1, i2, ilist.begin(), ilist.end()); }
#endif
		//21.4.6.7 copy()
		size_type copy(char* s, size_type n, size_type pos = 0) const;
		//21.4.6.8 swap()
		void swap(u8string& s) { str_.swap(s.str_); on_change_(); s.on_change_(); return; }

		/*******************
		* 21.4.7, string operations
		*******************/
		//21.4.7.1 accessors
		const char* data() const { return str_.c_str(); }
		const char* c_str() const { return str_.c_str(); }
        explicit operator const char*() const { return str_.c_str(); }
		const std::string& str() const { return str_; }

		size_type find(const u8string& str, size_type pos = 0) const noexcept{ return str_.find(str.str_, pos); }
		size_type find(const char* s, size_type pos, size_type n) const { return find(u8string(s, n), pos); }
		size_type find(const char* s, size_type pos = 0) const { return find(u8string(s), pos);	}
		template<typename charT>
		size_type find(charT c, size_type pos = 0) const { return find(u8string(1, c), pos); }

		size_type rfind(const u8string& str, size_type pos = npos) const noexcept{ return str_.rfind(str.str_, pos); }
		size_type rfind(const char* s, size_type pos, size_type n) const { return rfind(u8string(s, n), pos); }
		size_type rfind(const char* s, size_type pos = npos) const { return rfind(u8string(s), pos); }
		template<typename charT>
		size_type rfind(charT c, size_type pos = npos) const { return rfind(u8string(1, c), pos); }

		size_type find_first_of(const u8string& str, size_type pos = 0) const noexcept{ return str_.find_first_of(str.str_, pos); }
		size_type find_first_of(const char* s, size_type pos, size_type n) const { return find_first_of(u8string(s, n), pos); }
		size_type find_first_of(const char* s, size_type pos = 0) const { return find_first_of(u8string(s), pos); }
		template<typename charT>
		size_type find_first_of(charT c, size_type pos = 0) const { return find_first_of(u8string(1, c), pos); }

		size_type find_last_of(const u8string& str, size_type pos = npos) const noexcept{ return str_.find_last_of(str.str_, pos); }
		size_type find_last_of(const char* s, size_type pos, size_type n) const { return find_last_of(u8string(s, n), pos); }
		size_type find_last_of(const char* s, size_type pos = npos) const { return find_last_of(u8string(s), pos); }
		template<typename charT>
		size_type find_last_of(charT c, size_type pos = npos) const { return find_last_of(u8string(1, c), pos); }

		size_type find_first_not_of(const u8string& str, size_type pos = 0) const noexcept{ return str_.find_first_not_of(str.str_, pos); }
		size_type find_first_not_of(const char* s, size_type pos, size_type n) const { return find_first_not_of(u8string(s, n), pos); }
		size_type find_first_not_of(const char* s, size_type pos = 0) const { return find_first_not_of(u8string(s), pos); }
		template<typename charT>
		size_type find_first_not_of(charT c, size_type pos = 0) const { return find_first_not_of(u8string(1, c), pos); }

		size_type find_last_not_of(const u8string& str, size_type pos = npos) const noexcept{ return str_.find_last_not_of(str.str_, pos); }
		size_type find_last_not_of(const char* s, size_type pos, size_type n) const { return find_last_not_of(u8string(s, n), pos); }
		size_type find_last_not_of(const char* s, size_type pos = npos) const { return find_last_not_of(u8string(s), pos); }
		template<typename charT>
		size_type find_last_not_of(charT c, size_type pos = npos) const { return find_last_not_of(u8string(1, c), pos); }

		u8string substr(size_t pos = 0, size_t cnt = -1) const {
			//cnt = std::min(cnt, str_.size());
			return u8string(str_.substr(pos, cnt));
		}

		int compare(const u8string& str) const noexcept	{ return str_.compare(str.str_); }
		int compare(size_type pos1, size_type n1, const u8string& str) const { return u8string(*this, pos1, n1).compare(str); }
		int compare(size_type pos1, size_type n1, const u8string& str, size_type pos2, size_type n2) const { 
			return u8string(*this, pos1, n1).compare(u8string(str, pos2, n2)); 
		}
		template<typename charT>
		int compare(const charT* s) const { return compare(u8string(s)); }
		template<typename charT>
		int compare(size_type pos1, size_type n1, const charT* s) const { return u8string(*this, pos1, n1).compare(s); }
		template<typename charT>
		int compare(size_type pos1, size_type n1, const charT* s, size_type n2) const {
			return u8string(*this, pos1, n1).compare(u8string(s, n2));
		}
		
		/*************************
		 * other non-standard utilities;
		 **************************/
		//const
#ifdef _HAS_CHAR_T_SUPPORT
		std::u16string u16string() const { return unicode::u16u8_cvt.from_bytes(str_); }
		std::u32string u32string() const { return unicode::u32u8_cvt.from_bytes(str_); }
#endif
		std::wstring wstring() const { return unicode::wcu8_cvt.from_bytes(str_); }
		bool is_valid() const { return unicode::validate(str_.data()); }
		bool is_null() const { return empty();	}
		bool startswith(u8string const& s) const { return (s.size() > size()) ? false : (compare(0, s.size(), s) == 0); }
		bool endswith(u8string const& s) const { return (s.size() > size()) ? false : (compare(size() - s.size(), s.size(), s) == 0); }
		u8string trim_left(u8string const& s) const;
		u8string trim_right(u8string const& s) const;
		u8string trim(u8string const& s) const { return trim_left(s).trim_right(s);	}
		u8string trim_spaces(std::locale const& loc = std::locale("")) const ;
		u8string quote(u8string const& q = "\"", u8string const& escape = "\"") const ;
		u8string unquote(u8string const& q = "\"", u8string const& escape = "\"") const ;
		std::vector<u8string> split(bool raw_item, u8string const& separator, u8string const& quote, u8string const& escape) const;
		std::vector<u8string> split(bool raw_item = true, u8string const& s = ",", u8string const& q = "\"") const { return split(raw_item, s, q, q); }
		u8string join(std::vector<u8string> const& items, u8string const& quote, u8string const& escape) const;
		template<typename Iter>
		u8string join(Iter begin, Iter end, u8string const& quote, u8string const& escape) const {
			u8string ret;
			size_type sz = std::distance(begin, end);
			Iter iter = begin;
			for (size_type i = 0; i < sz - 1; ++i, ++iter) {
				ret.append((*iter).quote(quote, escape));
				ret.append(*this);
			}
			ret.append((*iter).quote(quote, escape));
			return ret;
		}
		u8string toupper(std::locale const& loc = std::locale("")) const;
		u8string tolower(std::locale const& loc = std::locale("")) const;

		//non-const;
		u8string& format(size_type max, const char* fmt, ...);
		u8string& format(size_type max, const char* fmt, va_list arg_list);
		u8string& append(size_type max, const char* fmt, ...);
		u8string& append(size_type max, const char* fmt, va_list arg_list);
		int replace_all(u8string const& from, u8string const& to);

#if _ZKS_U8STRING_INDEX
		size_type u8_size() const { update_(); return index_.size() - 1; }
		size_type u8_length() const { return u8_size(); }
		size_type u8_distance(size_type p1, size_type p2) const;
		size_type u8_distance(size_type p) const { return u8_distance(p, p+1); }

		std::vector<size_type> const& u8_index() const { update_(); return index_; }
		size_type u8_index(size_type n) const { update_(); return index_[n]; }
		size_type u8_index_last() const { return index_[u8_size() - 1]; }
		u8_iterator u8_at(size_type n) { on_change_(); return u8_iterator(&str_[u8_index(n)]); }
		const_u8_iterator u8_at(size_type n) const { return const_u8_iterator(&str_[u8_index(n)]); }
		const_u8_iterator u8_front() const { update_(); return const_u8_iterator(&str_[0]); }
		const_u8_iterator u8_back() const { return const_u8_iterator(&str_[u8_index_last()]); }

		u8string& u8_append(u8string const& str, size_type p = 0, size_type c = npos);
		u8string& u8_insert(size_type p1, u8string const& str2, size_type p2 = 0, size_type c2 = npos);
		u8string& u8_erase(size_type p, size_type c);
		u8string& u8_truncate(size_type p) { return u8_erase(p, npos); }
		u8string& u8_replace(size_type p1, size_type c1, u8string const& str2, size_type p2 = 0, size_type c2 = npos);
		u8string u8_substr(size_type n, size_type c);
#endif

#ifdef _HAS_WIN32_CSTRING
		CString get_CString() const;
		u8string& operator=(CString const& cstr);
#endif
	}; /* class u8string */


	/*******************
	* 21.4.8, non-member functions
	*******************/
	inline std::ostream& operator<<(std::ostream& os, u8string const& u) {
		os << u.c_str();
		return os;
	}

	template<typename T_>
	inline u8string to_u8string(T_ const& v) {
		return u8string(std::to_string(v));
	}
	template<typename charT>
	inline std::ostream& operator<<(std::ostream& os, u8_iterator_<charT> const& u) {
		os << u8string(u);
		return os;
	}

	inline u8string operator+(const u8string& lhs, const u8string& rhs) {	return u8string(lhs).append(rhs); }
	inline u8string operator+(u8string&& lhs, const u8string& rhs) { return std::move(lhs.append(rhs)); }	
	inline u8string operator+(const u8string& lhs, u8string&& rhs) { return std::move(rhs.insert(0, lhs)); }	
	inline u8string operator+(u8string&& lhs, u8string&& rhs) { return std::move(lhs.append(rhs)); }	
	//charT*
	template<typename charT>
	u8string operator+(const charT* lhs, const u8string& rhs) {
		u8string::size_type sz = std::char_traits<charT>::length(lhs);
		return u8string(lhs, lhs + sz).append(rhs);
	}
	template<typename charT>
	u8string operator+(const charT* lhs, u8string&& rhs) {
		u8string::size_type sz = std::char_traits<charT>::length(lhs);
		return std::move(rhs.insert(0, u8string(lhs, lhs + sz)));
	}
	template<typename charT>
	u8string operator+(const u8string& lhs, const charT* rhs)  {
		u8string::size_type sz = std::char_traits<charT>::length(rhs);
		return lhs + u8string(rhs, rhs + sz);
	}
	template<typename charT>
	u8string operator+(u8string&& lhs, const charT* rhs) {
		u8string::size_type sz = std::char_traits<charT>::length(rhs);
		return std::move(lhs.append(u8string(rhs, rhs + sz)));
	}
	//charT
	template<typename charT>
	u8string operator+(charT lhs, const u8string& rhs) {
		return std::move(u8string(1, lhs).append(rhs));
	}
	template<typename charT>
	u8string operator+(charT lhs, u8string&& rhs) {
		return std::move(u8string(1, lhs).append(rhs));
	}
	template<typename charT>
	u8string operator+(const u8string& lhs, charT rhs) {
		return lhs + u8string(1, rhs);
	}
	template<typename charT>
	u8string operator+(u8string&& lhs, charT rhs) {
		return std::move(lhs.append(rhs));
	}
	
	inline bool operator==(const u8string& lhs, const u8string& rhs) noexcept{
		return lhs.compare(rhs) == 0;
	}
	template<typename charT>
	bool operator==(const charT* lhs, const u8string& rhs) {
		return rhs.compare(lhs) == 0;
	}
	template<typename charT>
	bool operator==(const u8string& lhs, const charT* rhs) {
		return lhs.compare(rhs) == 0;
	}
	
	inline bool operator!=(const u8string& lhs, const u8string& rhs) noexcept{
		return lhs.compare(rhs) != 0;
	}
	template<typename charT>
	bool operator!=(const charT* lhs, const u8string& rhs) {
		return rhs.compare(lhs) != 0;
	}
	template<typename charT>
	bool operator!=(const u8string& lhs, const char* rhs) {
		return lhs.compare(rhs) != 0;
	}
	
	inline bool operator<(const u8string& lhs, const u8string& rhs) noexcept{
		return lhs.compare(rhs) < 0;
	}
	template<typename charT>
	bool operator<(const charT* lhs, const u8string& rhs) {
		return rhs.compare(lhs) < 0;
	}
	template<typename charT>
	bool operator<(const u8string& lhs, const char* rhs) {
		return lhs.compare(rhs) < 0;
	}
	
	inline bool operator>(const u8string& lhs, const u8string& rhs) noexcept{
		return lhs.compare(rhs) > 0;
	}
	template<typename charT>
	bool operator>(const charT* lhs, const u8string& rhs) {
		return rhs.compare(lhs) > 0;
	}
	template<typename charT>
	bool operator>(const u8string& lhs, const char* rhs) {
		return lhs.compare(rhs) > 0;
	}
	
	inline bool operator<=(const u8string& lhs, const u8string& rhs) noexcept{
		return lhs.compare(rhs) <= 0;
	}
	template<typename charT>
	bool operator<=(const charT* lhs, const u8string& rhs) {
		return rhs.compare(lhs) <= 0;
	}
	template<typename charT>
	bool operator<=(const u8string& lhs, const char* rhs) {
		return lhs.compare(rhs) <= 0;
	}
	
	inline bool operator>=(const u8string& lhs, const u8string& rhs) noexcept{
		return lhs.compare(rhs) >= 0;
	}
	template<typename charT>
	bool operator>=(const charT* lhs, const u8string& rhs) {
		return rhs.compare(lhs) >= 0;
	}
	template<typename charT>
	bool operator>=(const u8string& lhs, const char* rhs) {
		return lhs.compare(rhs) >= 0;
	}

	//template<typename charT>
	//u8string operator "" _u8s(const char* str, size_t len) {
	//	return u8string(str, len);
	//}

	// 21.4.8.8, swap:
	inline void swap(u8string& lhs, u8string& rhs) { lhs.swap(rhs); }

	enum txt_format { utf16le, utf16be, utf8bom, utf8, unknown };
	txt_format txt_peek_header(u8string const& fn);

    u8string decode(const char* loc_name, std::string const& str);
    std::string encode(const char* loc_name, u8string const& u8str);

	

} /*namespace zks*/

namespace std {
	template<>
	struct hash<zks::u8string>
	{
		typedef zks::u8string argument_type;
		typedef std::size_t value_type;

		value_type operator()(argument_type const& s) const
		{
			return std::hash<std::string>()(s.data());
		}
	};
}


#endif /*_JZS_U8STRING_H*/
