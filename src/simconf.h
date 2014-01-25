#ifndef SIMCONF_H_
#define SIMCONF_H_

#include "u8string.h"

#include <stdexcept>
#include <iostream>
#include <fstream>

#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace zks {

	class simconf {
	public:
		//typedef std::map<u8string, u8string> OptionMap;
		//typedef std::map<u8string, OptionMap> SectionMap;
		typedef std::unordered_map<u8string, u8string> OptionMap;
		typedef std::unordered_map<u8string, OptionMap> SectionMap;

	private:
		u8string _sep_, _quote_, _escape_; //for option values;
		u8string _assign_, _cmt_;// key-word [_assign_] values [_cmt_] comments;
		u8string _fname_;
		std::unordered_set<u8string> _fname_set_;
		u8string _sec_name_;
		SectionMap _sec_map_;

		mutable std::istringstream _iss_;

		void _del_comments_(u8string& line);
		int _get_sec_(const u8string& line, u8string* res);
		int _replace_var_(u8string* line, std::ostream* iop = 0);

		struct _FnameGuard {
			std::unordered_set<u8string>& _set_;
			u8string const& _fn_;
			_FnameGuard(std::unordered_set<u8string>& set, u8string const& fn) : _set_(set), _fn_(fn) {
				if (_set_.find(_fn_) != _set_.end()) {
					throw std::runtime_error("recursively include file.");
				}
				_set_.insert(_fn_);
			}
			~_FnameGuard() {
				_set_.erase(_fn_);
			}
		};
	public:
		simconf(u8string const& assign = "=", u8string const& sep = ",", u8string const & cmt = "#") :
			_sep_(sep), _assign_(assign), _cmt_(cmt), _sec_name_("global") {
			_quote_ = R"(")";
			_escape_ = R"(\)";
			_sec_map_[_sec_name_].clear();
		}
		~simconf() {
		}

		int parse(u8string const& file_name, std::ostream* iop = 0);
		SectionMap const& data() const {
			return _sec_map_;
		}
		bool has_option(u8string const& sec, u8string const& opt) const {
			return _sec_map_.count(sec) && _sec_map_.at(sec).count(opt);
		}

		int section(const u8string& sec, OptionMap* res) const;

		int option(const u8string& sec, const u8string& opt, u8string* res) const;

		int option_char(const u8string& sec, const u8string& opt, char* res) const;

		int option_vec(const u8string& sec, const u8string& opt, std::vector<u8string>* res) const;

		int option_set(const u8string& sec, const u8string& opt, std::unordered_set<u8string>* res) const;

		template<typename T_>
		int option_num(const u8string& sec, const u8string& opt, T_* res) const {
			if (!res) {
				return -99;
			}
			int ret;
			u8string value;
			ret = option(sec, opt, &value);
			if (ret < 0) {
				return ret;
			}
			_iss_.clear();
			_iss_.str(value.data());
			_iss_ >> (*res);
			if (_iss_.fail() && !_iss_.eof()) {
				return -1;
			}
			else if (_iss_.bad()) {
				return -2;
			}
			else if (!_iss_.eof()) {
				return -3;
			}
			return 0;
		}
	};

}

#endif /*SIMCONF_H_*/
