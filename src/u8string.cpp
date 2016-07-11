#include "u8string.h"

#include <cstdio>
#include <cstdarg>
#include <cassert>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <fstream>

namespace zks
{

    namespace unicode
    {
#ifdef _HAS_CHAR_T_SUPPORT
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> u16u8_cvt;
        std::wstring_convert<std::codecvt_utf8<char32_t, 0x7fffffff>, char32_t> u32u8_cvt;
#endif
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcu8_cvt;

        char16_t to_char16(const char* u)
        {
            if ((u[0] & 0x80) && u[1]) {
                if ((u[0] & 0x20) && u[2]) {
                    return (char16_t) (((u[0] & 0x0f) << 12) | ((u[1] & 0x3f) << 6) | (u[2] & 0x3f));
                }
                else {
                    return (char16_t) (((u[0] & 0x1f) << 6) | (u[1] & 0x3f));
                }
            }
            else {
                return (unsigned char) u[0];
            }
        }

        bool validate_u8char(const char* s, size_t* pLen)
        {
            if (!s) {
                if (pLen)
                    *pLen = 0;
                return false;
            }
            if (!(*s & 0x80)) {
                if (pLen)
                    *pLen = 1;
                return true;
            }

            /**
             * if (lead_n) bit is lead_control, it's using (lead_n-1) bytes in sequence.
             * Thus (lead_n-2) is the last byte in continuation bytes(cb).
             */
            bool ret { false };
            size_t len { 0 };
            char lead_mask { (char) 0xe0 }, lead_control { (char) 0xc0 };
            for (int lead_n = 3; lead_n < 8; ++lead_n) {
                if ((*s & lead_mask) == lead_control) {
                    len = lead_n - 1;
                    size_t b = 1;
                    while (b < len && s[b] && (s[b] & 0xc0) == 0x80)
                        ++b;
                    assert(b <= len);
                    if (b < len) {
                        len = b + 1;
                        break;
                    }
                    if (len == 3) {
                        char16_t wc = to_char16(s);
                        if (invalid_codepoint(wc))
                            break; //valid utf8 code but invalid unicode cp. code point in this area are reserved for utf16.
                    }
                    ret = true;
                    break;
                }
                lead_control = lead_mask;
                lead_mask |= 0x01 << (7 - lead_n);
            }

            if (pLen)
                *pLen = len;
            return ret;
        }

        bool validate_range(const char* i1, const char* i2, size_t* pLen)
        {
            if (!i1 || !i2 || i1 > i2) {
                return false;
            }
            size_t char8_len { 0 }, u8len { 0 };
            const char* i = i1;
            while (i < i2 && *i && validate_u8char(i, &char8_len)) {
                i += char8_len;
                u8len += char8_len;
            }
            pLen ? (*pLen = u8len) : 0;
            return i == i2;
        }
    }

    int u8string::update_() const
    {
#if _ZKS_U8STRING_INDEX
        if (!changed_) {
            return 0;
        }

        size_type char8_len { 0 }, index { 0 };
        const char* s = str_.c_str();
        index_.push_back(index); // 0 for the 1st u8char;
        while (*s && unicode::validate_u8char(s, &char8_len)) {
            s += char8_len;
            index += char8_len;
            index_.push_back(index);
        }

        if (index != str_.size()) {
#ifndef _ZKS_U8STRING_NOVALIDATION
            std::string err_msg {"input utf8 string is invalid at position("};
            err_msg += std::to_string(index) + ")";
            throw std::runtime_error(err_msg.c_str());
#else
            return -1;
#endif
        }

        changed_ = false;
        is_valid_ = true;
        return (int)index_.size();
#else
        return 0;
#endif
    }

    u8string& u8string::append(const char* s, size_type n)
    {
        n = std::min(n, traits_type::length(s));
#ifndef _ZKS_U8STRING_NOVALIDATION
        if (!unicode::validate_range(s, s + n)) {
            throw std::invalid_argument("string to be added is invalid utf8");
        }
#endif
        str_.append(s, n);
        on_change_();
        return *this;
    }

#ifdef _HAS_CHAR_T_SUPPORT
    u8string& u8string::append(size_type n, char16_t c16) {
        std::string u = unicode::u16u8_cvt.to_bytes(c16);
        reserve(size() + n * u.size());
        for (size_type i = 0; i < n; ++i) {
            append(u.data());
        }
        return *this;
    }

    u8string& u8string::append(size_type n, char32_t c32) {
        std::string u = unicode::u32u8_cvt.to_bytes(c32);
        reserve(size() + n * u.size());
        for (size_type i = 0; i < n; ++i) {
            append(u.data());
        }
        return *this;
    }
#endif

    u8string& u8string::append(size_type n, wchar_t wc)
    {
        std::string u = unicode::wcu8_cvt.to_bytes(wc);
        reserve(size() + n * u.size());
        for (size_type i = 0; i < n; ++i) {
            append(u.data());
        }
        return *this;
    }

    u8string& u8string::assign(const char* s, size_type pos, size_type cnt)
    {
        if (!s || cnt == 0) {
            return *this;
        }
        size_type sz = traits_type::length(s);
        cnt = std::min(cnt, size_type(-1) - pos);
        sz = std::min(sz, pos + cnt);
        cnt = std::min(sz - pos, cnt);

        str_.resize(cnt);
        std::copy(s + pos, s + pos + cnt, std::begin(str_));

#ifndef _ZKS_U8STRING_NOVALIDATION
        size_type len {0};
        if (!unicode::validate(str_.c_str(), &len)) {
            std::string err_msg {"input utf8 string is invalid at position("};
            err_msg += std::to_string(len) + ")";
            throw std::invalid_argument(err_msg.c_str());
        }
#endif

        on_change_();
        return *this;
    }

    u8string& u8string::assign(size_type s, const char* str)
    {
        if (!str || s == 0) {
            return *this;
        }
        size_type sz { traits_type::length(str) };
#ifndef _ZKS_U8STRING_NOVALIDATION
        if (!unicode::validate(str)) {
            throw std::invalid_argument("input string is nullptr or invalid.");
        }
#endif
        s *= sz;
        str_.resize(s);
        for (size_type i = 0; i < s; ++i) {
            str_[i] = str[i % sz];
        }

        on_change_();
        return *this;
    }

    u8string& u8string::insert(size_type index, size_type count, const char* str, size_type str_cnt)
    {
        if (!str || count == 0 || str_cnt == 0) {
            return *this;
        }
        if (index > size()) {
            throw std::range_error("index is beyond the u8string size.");
        }

        size_type str_sz = traits_type::length(str);
        str_cnt = std::min(str_sz, str_cnt);
#ifndef _ZKS_U8STRING_NOVALIDATION
        if (!unicode::validate(std::string(str, str_cnt).data())) {
            throw std::invalid_argument("input string is nullptr or invalid.");
        }
#endif
        str_sz = count * str_cnt;
        if (size() + str_sz >= max_size()) {
            throw std::length_error("new string is too long.");
        }
        resize(size() + str_sz);
        for (auto p = size() - 1; p >= index + str_sz; --p) {
            str_[p] = str_[p - str_sz];
        }

        for (size_type i = 0; i < str_sz; ++i) {
            str_[index + i] = str[i % str_cnt];
        }

        on_change_();
        return *this;
    }

    u8string& u8string::replace(size_type pos, size_type n1, const char* s, size_type n2)
    {
        if (pos >= size()) {
            throw std::range_error("index is beyond the u8string size.");
        }
        u8string add_str { s, n2 };
        n1 = std::min(n1, size() - pos);
#ifndef _ZKS_U8STRING_NOVALIDATION
        if (!add_str.is_valid() || !unicode::validate_range(&str_[pos], &str_[pos + n1])) {
            throw std::invalid_argument("new string is invalid utf8 string");
        }
#endif
        size_type new_sz = size() - n1 + add_str.size();
        if (new_sz >= max_size()) {
            throw std::length_error("new string is too long.");
        }
        //reserve(new_sz);
        //erase(pos, n1);
        //insert(pos, add_str);
        u8string res;
        res.reserve(new_sz);
        res.assign(*this, 0, pos);
        res.append(add_str);
        res.append(*this, pos + n1, npos);

        assign(std::move(res));
        on_change_();
        return *this;
    }

    u8string::size_type u8string::copy(char* s, size_type n, size_type pos) const
    {
        if (!s || n == 0) {
            return 0;
        }
        if (pos > size()) {
            throw std::out_of_range("copy from invalid position.");
        }
        n = std::min(n, size() - pos);
        std::copy(&str_[pos], &str_[pos + n], s);
        return n;
    }

    u8string& u8string::format(size_type max, const char* fmt, ...)
    {
        std::unique_ptr<char> buff { new char[max] };
        va_list arg_list;
        va_start(arg_list, fmt);
        int sz = std::vsnprintf(buff.get(), max, fmt, arg_list);
        va_end(arg_list);

        if (sz < 0) {
            return *this;
        } else if((size_t) sz > max) {
            sz = (int)max;
        }
        assign(buff.get(), sz);
        on_change_();
        return *this;
    }

    u8string& u8string::format(size_type max, const char* fmt, va_list arg_list)
    {
        std::unique_ptr<char> buff{ new char[max] };
        int sz = std::vsnprintf(buff.get(), max, fmt, arg_list);
        if (sz < 0) {
            return *this;
        }
        else if ((size_t)sz > max) {
            sz = (int)max;
        }
        assign(buff.get(), sz);
        on_change_();
        return *this;
    }

    u8string& u8string::append(size_type max, const char* fmt, ...)
    {
        std::unique_ptr<char> buff{ new char[max] };
        va_list arg_list;
        va_start(arg_list, fmt);
        int sz = std::vsnprintf(buff.get(), max, fmt, arg_list);
        va_end(arg_list);

        if (sz < 0) {
            return *this;
        }
        else if ((size_t)sz > max) {
            sz = (int)max;
        }
        append(buff.get(), sz);
        on_change_();
        return *this;
    }

    u8string& u8string::append(size_type max, const char* fmt, va_list arg_list)
    {
        std::unique_ptr<char> buff{ new char[max] };
        int sz = std::vsnprintf(buff.get(), max, fmt, arg_list);
        if (sz < 0) {
            return *this;
        }
        else if ((size_t)sz > max) {
            sz = (int)max;
        }
        append(buff.get(), sz);
        on_change_();
        return *this;
    }

    int u8string::replace_all(u8string const& from, u8string const& to)
    {
        if (from.is_null()) {
            return 0;
        }

        int ret { 0 };
        size_t start(0), end(0), flen(from.size()), tlen(to.size());
        while ((start = find(from, end)) != npos) {
            replace(start, flen, to);
            ++ret;
            end = start + tlen;
        }
        return ret;
    }

    u8string u8string::trim_left(u8string const& s) const
    {
        u8string ret { *this };
        if (ret.startswith(s)) {
            ret.erase(0, s.size());
        }
        return ret;
    }

    u8string u8string::trim_right(u8string const& s) const
    {
        u8string ret { *this };
        if (ret.endswith(s)) {
            ret.erase(size() - s.size());
        }
        return ret;
    }

    u8string u8string::trim_spaces() const {
        u8string ret{ *this };
        size_t p = ret.find_first_not_of(" \t\r\n\f\v");
        if (p != ret.npos){
            ret = ret.substr(p);
        }
        p = ret.find_last_not_of(" \t\r\n\f\v");
        if (p < ret.size()-1) {
            ret = ret.substr(0, p + 1);
        }
        return ret;
    }
    u8string u8string::trim_spaces(std::locale const& loc) const
    {
        u8string ret { *this };
        if (ret.is_null()) {
            return ret;
        }
        size_type p { 0 };
        while (p < ret.size() && std::isspace(ret[p], loc))
            ++p;
        ret.erase(0, p);

        if (ret.is_null()) {
            return ret;
        }
        long pp = (long) ret.size() - 1;
        while (pp >= 0 && std::isspace(ret[pp], loc))
            --pp;
        ret.erase(pp + 1);

        return ret;
    }

    u8string u8string::quote(u8string const& q, u8string const& escape) const
    {
        size_type p { 0 }, sz(size()), qsz(q.size());
        u8string ret;
        ret.reserve(2 * size()); //magic number, experience value; could be tuned;

        ret.append(q);
        while (p < sz) {
            if (qsz && p + qsz <= sz && compare(p, qsz, q) == 0) {
                ret.append(escape);
                ret.append(q);
                p += qsz;
            }
            else {
                ret.append(str_[p]);
                ++p;
            }
        }
        ret.append(q);

        return ret;
    }

    u8string u8string::unquote(u8string const& q, u8string const& escape) const
    {
        size_type p { 0 }, end { size() }, qsz(q.size()), esz(escape.size());
        if (!startswith(q) || !endswith(q) || end < qsz || compare(q) == 0 || qsz == 0) {
            return *this;
        }

        u8string t { escape + q };
        u8string res;
        res.reserve(end);
        p += qsz;
        end -= qsz;
        while (p < end) {
            if (p + esz + qsz > end) {
                res.append(&str_[p], &str_[end]);
                p = end;
            }
            else if (p + esz + qsz <= end && compare(p, esz + qsz, t) == 0) {
                res.append(q);
                p += esz + qsz;
            }
            else {
                res.append(str_[p]);
                ++p;
            }
        }

        return res;
    }

    std::vector<u8string> u8string::split(bool raw_item, u8string const& separator, u8string const& quote, u8string const& escape) const
    {
        std::vector<u8string> ret;
        size_type p { 0 }, ihead { 0 }, sz(size()), ssz(separator.size()), qsz(quote.size()), esz(escape.size());
        bool in_quote { false };
        while (p < sz) {
            if (!in_quote && p + ssz <= sz && equal_to(p, ssz, separator)) { //if it's a separator that not in quote;
                if (raw_item) {
                    ret.emplace_back(str_.data(), ihead, p - ihead);
                }
                else {
                    ret.push_back(u8string{ str_.data(), ihead, p - ihead }.trim_spaces().unquote(quote, escape));
                }
                ihead = ++p;
                continue;
            }
            if (esz && p + esz <= sz && equal_to(p, esz, escape)) { //if it's a escape char,
                if (p + esz + qsz < sz && equal_to(p + esz, qsz, quote)) { //and followed by a non-ending-quote;
                    p += esz + qsz;
                    continue;
                }
            }
            if (qsz && p + qsz <= sz && equal_to(p, qsz, quote)) {
                in_quote = !in_quote;
            }
            ++p;
        }
        if (ihead < sz) {
            if (raw_item) {
                ret.emplace_back(str_.data(), ihead, sz - ihead);
            }
            else {
                ret.push_back(u8string{ str_.data(), ihead, sz - ihead }.trim_spaces().unquote(quote, escape));
            }
		}
		else if (ihead == sz) {
			ret.push_back("");
		}
		return	ret;
    }

    u8string u8string::join(std::vector<u8string> const& items, u8string const& quote, u8string const& escape) const
    {
        u8string ret;
        size_type sz = items.size();
        for (size_type i = 0; i < sz - 1; ++i) {
            ret.append(items[i].quote(quote, escape));
            ret.append(*this);
        }
        ret.append(items.back().quote(quote, escape));
        return ret;
    }

    u8string u8string::toupper(std::locale const& loc) const
    {
        u8string ret;
        ret.reserve(size());
        for (auto i = u8_cbegin(); i != u8_cend(); ++i) {
            for (auto c : unicode::wcu8_cvt.from_bytes(i.begin(), i.end())) {
                ret.append(std::toupper(c, loc));
            }
        }

        return ret;
    }

    u8string u8string::tolower(std::locale const& loc) const
    {
        u8string ret;
        ret.reserve(size());
        for (auto i = u8_cbegin(); i != u8_cend(); ++i) {
            for (auto c : unicode::wcu8_cvt.from_bytes(i.begin(), i.end())) {
                ret.append(std::tolower(c, loc));
            }
        }

        return ret;
    }

#if _ZKS_U8STRING_INDEX
    u8string::size_type u8string::u8_distance(size_type p1, size_type p2) const
    {
        if (p1 >= index_.size() || p2 >= index_.size()) {
            throw std::out_of_range("invalid u8 index range.");
        }
        auto p = std::minmax(p1, p2);
        return index_[p.second] - index_[p.first];
    }

    u8string& u8string::u8_append(u8string const& str, size_type p, size_type c)
    {
        if (p > str.u8_size()) {
            throw std::out_of_range("invalid u8 position.");
        }
        c = std::min(c, str.u8_size() - p);
        if (c == 0) {
            return *this;
        }

        return append(str, str.u8_index(p), str.u8_distance(p, p + c));
    }
    u8string& u8string::u8_insert(size_type p1, u8string const& str2, size_type p2, size_type c2)
    {
        if (p2 > str2.u8_size()) {
            throw std::out_of_range("invalid u8 position.");
        }
        if (p1 > u8_size()) {
            throw std::out_of_range("invalid u8 position.");
        }
        c2 = std::min(c2, str2.u8_size() - p2);
        if (c2 == 0) {
            return *this;
        }

        return insert(u8_index(p1), str2, str2.u8_index(p2), str2.u8_distance(p2, p2 + c2));
    }
    u8string& u8string::u8_erase(size_type p, size_type c)
    {
        if (p > u8_size()) {
            throw std::out_of_range("invalid u8 position.");
        }
        c = std::min(c, u8_size() - p);
        if (c == 0) {
            return *this;
        }

        return erase(u8_index(p), u8_distance(p, p + c));
    }
    u8string& u8string::u8_replace(size_type p1, size_type c1, u8string const& str2, size_type p2, size_type c2)
    {
        if (p1 > u8_size()) {
            throw std::out_of_range("invalid u8 position.");
        }
        c1 = std::min(c1, u8_size() - p1);
        if (p2 > str2.u8_size()) {
            throw std::out_of_range("invalid u8 position.");
        }
        c2 = std::min(c2, str2.u8_size() - p2);
        if (c2 == 0) {
            return *this;
        }

        return replace(u8_index(p1), u8_distance(p1, p1 + c1), str2, str2.u8_index(p2), str2.u8_distance(p2, p2 + c2));
    }
    u8string u8string::u8_substr(size_type p, size_type c)
    {
        if (p > u8_size()) {
            throw std::out_of_range("invalid u8 position.");
        }
        c = std::min(c, u8_size() - p);
        return substr(u8_index(p), u8_distance(p, p + c));
    }
#endif

    namespace unicode
    {
		txt_format txt_consume_header(std::istream & is)
		{
			if (is.fail()) {
				return zks::unicode::txt_format::error;
			}
			char header[3] = { 0 };
			is.read(header, 3);
			if (is.gcount() == 3) {
				if (header[0] == (char)0xfe && header[1] == (char)0xff) {
					is.seekg(2);
					return zks::unicode::txt_format::utf16be;
				}
				else if (header[0] == (char)0xff && header[1] == (char)0xfe) {
					is.seekg(2);
					return zks::unicode::txt_format::utf16le;
				}
				else if (header[0] == (char)0xef && header[1] == (char)0xbb && header[2] == (char)0xbf) {
					return zks::unicode::txt_format::utf8bom;
				}
			}
			is.seekg(0);
			std::string line;
			std::getline(is, line);
			is.seekg(0);
			if (unicode::validate(line.data())) {
				return zks::unicode::txt_format::utf8;
			}
			return zks::unicode::txt_format::unknown;
		}
		txt_format txt_peek_header(u8string const& fn)
        {
            std::ifstream fin(fn.c_str(), std::ios_base::binary);
			return txt_consume_header(fin);
        }
    } /* namespace unicode */

} /* namespace zks */

#ifdef ZKS_OS_GNULINUX_
zks::u8string zks::unicode::decode(const char* loc_name, std::string const& str)
{
    zks::unicode::Mbwc_cvt iconv(new zks::unicode::Mbwc_codecvt(loc_name));
    std::wstring wstr = iconv.from_bytes(str);
    return zks::u8string { wstr };
}

std::string zks::unicode::encode(const char* loc_name, zks::u8string const& u8str)
{
    zks::unicode::Mbwc_cvt iconv(new zks::unicode::Mbwc_codecvt(loc_name));
    std::string ret = iconv.to_bytes(u8str.wstring());
    return ret;
}

#elif defined(ZKS_OS_WINDOWS_)
#define _AMD64_
#include <Stringapiset.h>

zks::u8string zks::unicode::decode(int cp, std::string const& str) {
    std::wstring wstr;
    wstr.resize(2 * str.size() + 1);
    int ok = ::MultiByteToWideChar(cp, 0, str.data(), -1, &wstr[0], (int)wstr.size());
    if (!ok) {
        return zks::u8string {""};
    }
    return zks::u8string {wstr};
}

std::string zks::unicode::encode(int cp, zks::u8string const& u8str) {
    std::wstring wstr = u8str.wstring();
    std::string ret;
    int len = ::WideCharToMultiByte(cp, 0, wstr.data(), -1, &ret[0], 0, NULL, NULL);
    if (len == 0) {
        return ret;
    }
    ret.resize(len, '\0');
    len = ::WideCharToMultiByte(cp, 0, wstr.data(), -1, &ret[0], len, NULL, NULL);
    if (len == 0) {
        return std::string {};
    }
    return ret;
}

#endif

