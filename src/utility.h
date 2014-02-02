#ifndef _ZKS_UTILITY_H
#define _ZKS_UTILITY_H

#include "u8string.h"

#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>

namespace zks
{
	template<typename T>
	class LocalBackup {
		T old_;
		T& ref_;
	public:
		LocalBackup(T& v) : old_(std::move(v)), ref_(v) {}
		~LocalBackup() {
			ref_ = std::move(old_);
		}
	};
}

namespace std {

	template<typename Tp_>
	std::string to_string(const std::vector<Tp_>& vec) {
		std::string ret;
		ret += "[";
		auto begin = vec.cbegin();
		auto end = vec.cend();
		for (auto iter = begin; iter != end; ++iter) {
			if (iter == begin) {
				ret += std::to_string(*iter);
			}
			else {
				ret += "," + std::to_string(*iter);
			}
		}
		ret += "]";
		return ret;
	}

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
		os << to_string(vec);
		return os;
	}

	template<typename Tp_>
	std::string to_string(const std::set<Tp_>& s) {
		std::string ret;
		ret += "[";
		auto begin = s.cbegin();
		auto end = s.cend();
		for (auto iter = begin; iter != end; ++iter) {
			if (iter == begin) {
				ret += std::to_string(*iter);
			}
			else {
				ret += "," + std::to_string(*iter);
			}
		}
		ret += "]";
		return ret;
	}

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const std::set<T>& ms_set) {
		os << to_string(ms_set);
		return os;
	}

	template<typename T1_, typename T2_>
	std::string to_string(std::ostream& os, const std::pair<T1_, T2_>& p) {
		return std::string(std::to_string(p.first) + ":" + std::to_string(p.second));
	}

	template<typename T1_, typename T2_>
	std::ostream& operator<<(std::ostream& os, const std::pair<T1_, T2_>& p) {
		os << to_string(p);
		return os;
	}

	template<typename T1, typename T2>
	std::string to_string(const std::map<T1, T2>& ms_map) {
		std::string ret;
		ret += "{";
		for (const auto& p : ms_map)
			ret += to_string(p) + ",";
		ret += "}";
		return ret;
	}

	template<typename T1, typename T2>
	std::ostream& operator<<(std::ostream& os, const std::map<T1, T2>& ms_map) {
		os << to_string(ms_map);
		return os;
	}

} /* std */

#endif // !_ZKS_UTILITY_H
