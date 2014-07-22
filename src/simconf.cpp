#include "simconf.h"

#include <iostream>
#include <fstream>

#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace zks
{

    void simconf::_del_comments_(u8string& line)
    {
        size_t res = line.find(_cmt_);
        if (u8string::npos != res) {
            line.erase(line.begin() + res, line.end());
        }
        return;
    }
    int simconf::_get_sec_(const u8string& line, u8string* res)
    {
        if (line.startswith("[")) {
            if (line.endswith("]")) {
                if (res) {
                    *res = line.substr(1, line.size() - 2).tolower();
                }
                return (int)res->size();
            }
            else {
                return -1;
            }
        }
        else {
            return 0;
        }
    }

    //TODO: var in var recursively?
    int simconf::_replace_var_(u8string* line, std::ostream* iop)
    {
        if (!line) {
            return -1;
        }

        size_t tail(0);
        u8string from, to;
        for (size_t head = line->find("$("); head != u8string::npos; head = line->find("$(")) {
            tail = line->find(")", head);
            if (tail == u8string::npos) {
                return -2;
            }
            from = line->substr(head, tail - head + 1);
            if (option(_sec_name_, from.substr(2, from.size() - 3), &to) < 0 && option("global", from.substr(2, from.size() - 3), &to) < 0) {
                return -3;
            }
            if (iop) {
                *iop << from << " -> " << to << "\n";
            }
            if (line)
                line->replace_all(from, to);
        }

        return 0;
    }

    int simconf::parse(const u8string& file, std::ostream* iop)
    {
        //open file;
        _fname_ = file;
        _FnameGuard Here(_fname_set_, _fname_);
        std::wifstream ifs(_fname_.c_str());
        if (ifs.fail()) {
            std::cerr << " can not open: `[" << _fname_ << "]`" << std::endl;
            return -1;
        }
        std::locale loc { ifs.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header> };
        ifs.imbue(loc);

        std::vector<u8string> str_vec;
        int ln(1);
        int ret(0);
        u8string line;
        for (std::wstring buff; getline(ifs, buff); ++ln) {
            line = buff;
            _del_comments_(line);
            line = line.trim_spaces();
            if (line.size() < 2) {
                continue;
            }

            //include
            if (line.tolower().startswith("include")) {
                std::vector<u8string> svec = line.split(false, " ");
                if (svec.size() != 2 || svec[0].tolower() != "include") {
                    std::cerr << _fname_ << "(" << ln << "): invalid `include` usage." << std::endl;
                    return -2;
                }
                if (iop) {
                    (*iop) << "to parse `" << svec[1] << "` ... " << std::endl;
                }
                ret = parse(svec[1], iop);
                _fname_ = file;
                if (ret < 0) {
                    return -3;
                }
                if (iop) {
                    (*iop) << "back to parse `" << _fname_ << "` ... " << std::endl;
                }
                continue;
            }

            //section;
            ret = _get_sec_(line, &_sec_name_);
            if (ret < 0) {
                std::cerr << _fname_ << "(" << ln << "): section conf is not valid." << std::endl;
                return -4;
            }
            if (ret > 0) {
                if (_sec_map_.find(_sec_name_) != _sec_map_.end() && _sec_name_ != "global") {
                    if (iop) {
                        (*iop) << _fname_ << "(" << ln << "): section [" << _sec_name_ << "] re-define." << std::endl;
                    }
                    return -5;
                }
                if (iop) {
                    (*iop) << "get section name: " << _sec_name_ << std::endl;
                }
                _sec_map_[_sec_name_].clear();
                continue;
            }

            //normal options;
            str_vec = line.split(false, _assign_, _quote_, _escape_);
            if (str_vec.size() != 2) {
                std::cerr << _fname_ << "(" << ln << "): option conf is not seperated by `" << _assign_ << "`." << std::endl;
                return -6;
            }
            else {
                if (_sec_map_[_sec_name_].find(str_vec[0]) != _sec_map_[_sec_name_].end()) {
                    if (iop) {
                        (*iop) << _fname_ << "(" << ln << "): option [" << _sec_name_ << "-" << str_vec[0] << "] re-define." << std::endl;
                    }
                    return -7;
                }
                ret = _replace_var_(&str_vec[1], iop);
                if (ret < 0) {
                    if (iop) {
                        (*iop) << _fname_ << "(" << ln << "): replace var failed: " << ret << std::endl;
                    }
                    return -8;
                }
                if (iop) {
                    (*iop) << "\t[" << _sec_name_ << "." << str_vec[0] << "]: " << str_vec[1] << std::endl;
                }
                _sec_map_[_sec_name_][str_vec[0]] = str_vec[1];
            }
        }

        return 0;
    }

    int simconf::section(const u8string& sec, OptionMap* res) const
    {
        if (!res) {
            return -99;
        }
        const SectionMap::const_iterator iter = _sec_map_.find(sec);
        if (iter == _sec_map_.end()) {
            return -2;
        }
        (*res) = iter->second;
        return 0;
    }

    int simconf::option(const u8string& sec, const u8string& opt, u8string* res) const
    {
        if (!res) {
            return -99;
        }
        const SectionMap::const_iterator sec_iter = _sec_map_.find(sec.tolower());
        if (sec_iter == _sec_map_.end()) {
            return -2;
        }
        const OptionMap::const_iterator opt_iter = sec_iter->second.find(opt);
        if (opt_iter == sec_iter->second.end()) {
            return -3;
        }
        (*res) = opt_iter->second;
        return 0;
    }

    int simconf::option_char(const u8string& sec, const u8string& opt, char* res) const
    {
        if (!res) {
            return -99;
        }
        u8string v;
        int ret = option(sec, opt, &v);
        if (ret < 0) {
            return ret;
        }
        if (v.size() < 1) {
            return -98;
        }
        (*res) = v[0];
        return 0;
    }
    int simconf::option_vec(const u8string& sec, const u8string& opt, std::vector<u8string>* res) const
    {
        if (!res) {
            return -99;
        }
        int ret;
        u8string value;
        ret = option(sec, opt, &value);
        if (ret < 0) {
            return ret;
        }
        (*res) = value.split(false, _sep_, _quote_, _escape_);
        return (int)res->size();
    }

    int simconf::option_set(const u8string& sec, const u8string& opt, std::unordered_set<u8string>* res) const
    {
        if (!res) {
            return -99;
        }
        std::vector<u8string> vec;
        int ret = option_vec(sec, opt, &vec);
        if (ret < 0) {
            return ret;
        }
        res->clear();
        for (size_t i = 0; i < vec.size(); ++i) {
            res->insert(vec[i]);
        }
        return (int)res->size();
    }

}
