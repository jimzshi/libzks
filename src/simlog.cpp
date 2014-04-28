#include "simlog.h"
#include "simconf.h"

#include <iostream>
#include <chrono>
#include <cstdarg>
#include <thread>
#include <fstream>

namespace zks {

	const std::array<u8string, simlog::Level::lsize> simlog::level_string = { {"EMERG", "FATAL", "ALERT", "CRIT", "ERROR", "WARN", "NOTICE", "INFO", "DEBUG", "TRACE"} };
	const std::array<u8string, simlog::Column::csize> simlog::column_string = { {"DATETIME", "EPOCHTIME", "THREAD", "FILE", "FUNC", "LINE", "GROUP", "LEVEL", "MESG"} };

	simlog::simlog(u8string const& ini_file) {
		if (configure(ini_file) < 0) {
			throw std::runtime_error("can not read in configuration file.");
		}
		reset();
	}
	simlog::simlog(int mode, u8string const& log_file) {
		config.stream_type = mode;
		config.output.file_name = log_file;
		reset();
	}

	void simlog::write(const char* file, const char* func, int line, u8string const& group, Level level, const char* fmt, ...) {
		if (level <= config.verbosity) {
			lock_guard_t here(mutex);

			std::va_list arg_list;
			va_start(arg_list, fmt);
			msg_buff_ = msg_buff_.format(config.buff.msg_max_size, fmt, arg_list).quote();
			va_end(arg_list);
			line_buff_.format(config.buff.line_buff_size, fmt_str_.c_str(),
				config.format.fieldset[Column::DATETIME] ? get_datetime_().c_str() : "",
				config.format.fieldset[Column::EPOCHTIME] ? (size_t) std::chrono::duration_cast<std::chrono::microseconds>(clock_t::now() - epoch_).count() : 0,
				config.format.fieldset[Column::THREAD] ? thread_hash_(std::this_thread::get_id()) : 0,
				config.format.fieldset[Column::FILE] ? file : "",
				config.format.fieldset[Column::FUNC] ? func : "",
				config.format.fieldset[Column::LINE] ? line : 0,
				config.format.fieldset[Column::GROUP] ? group.c_str() : "",
				config.format.fieldset[Column::LEVEL] ? level_string[level].c_str() : "",
				config.format.fieldset[Column::MESSAGE] ? msg_buff_.c_str() : ""
				);
			
			if (config.buff.enable) {
				buff_.append(line_buff_);
				if (!config.buff.enable || buff_.size() > config.buff.size) {
					flush();
				}
			}
			else {
				(*ostream_) << line_buff_ << std::flush;
			}
		}
	}

	u8string simlog::get_datetime_() {
		//locale_guard HERE(std::locale(config.format.locale.c_str()));
		std::time_t t = std::time(NULL);
		wchar_t wstr[100];
		if (std::wcsftime(wstr, 100, config.format.strftime.wstring().c_str(), std::localtime(&t))) {
			return u8string{ wstr };
		}
		else {
			return{ "bad_time" };
		}
	}

	void simlog::reset_buff_() {
		if (config.buff.enable) {
			buff_.clear();
			buff_.reserve(config.buff.size);
		}
		if (config.buff.enable_line_buff) {
			line_buff_.clear();
			line_buff_.reserve(config.buff.line_buff_size);
		}
		if (config.buff.enable_msg_buff) {
			msg_buff_.clear();
			msg_buff_.reserve(config.buff.msg_max_size);
		}
	}

	void simlog::reset_ostream_() {
		if (config.stream_type == 0) {
			ostream_ = std::make_shared<std::ostream>(std::cout.rdbuf());
		}
		else if (config.stream_type == 1) {
			ostream_ = std::make_shared<std::ostream>(std::cerr.rdbuf());
		}
		else if (config.stream_type == 2) {
			ostream_ = std::make_shared<std::ostream>(std::clog.rdbuf());
		}
		else if (config.stream_type == 3) {
			ostream_.reset(new std::ofstream(config.output.file_name.c_str(), std::ios_base::app));
			if (ostream_->bad()) {
				throw std::runtime_error("can not open log file.");
			}
		}
	}

	int simlog::configure(u8string const& ini_file) {
		simconf ini;
		int ret = ini.parse(ini_file);
		if (ret < 0) {
			return ret;
		}
		
		//global;
		if (ini.has_option("global", "stream_type")) {
			ini.option_num("global", "stream_type", &config.stream_type);
		}
		if (ini.has_option("global", "thread_safe")) {
			ini.option_num("global", "thread_safe", &config.thread_safe);
		}
		if (ini.has_option("global", "verbosity")) {
			ini.option_num("global", "verbosity", &config.verbosity);
		}
		//format
		if (ini.has_option("format", "strftime")) {
			ini.option("format", "strftime", &config.format.strftime);
		}
		if (ini.has_option("format", "seperator")) {
			ini.option("format", "seperator", &config.format.seperator);
		}
		if (ini.has_option("format", "quote")) {
			ini.option("format", "quote", &config.format.quote);
		}
		if (ini.has_option("format", "escape")) {
			ini.option("format", "escape", &config.format.escape);
		}
		if (ini.has_option("format", "datetime")) {
			ini.option("format", "datetime", &config.format.column[Column::DATETIME]);
		}
		if (ini.has_option("format", "epochtime")) {
			ini.option("format", "epochtime", &config.format.column[Column::EPOCHTIME]);
		}
		if (ini.has_option("format", "thread")) {
			ini.option("format", "thread", &config.format.column[Column::THREAD]);
		}
		if (ini.has_option("format", "file")) {
			ini.option("format", "file", &config.format.column[Column::FILE]);
		}
		if (ini.has_option("format", "func")) {
			ini.option("format", "func", &config.format.column[Column::FUNC]);
		}
		if (ini.has_option("format", "line")) {
			ini.option("format", "line", &config.format.column[Column::LINE]);
		}
		if (ini.has_option("format", "group")) {
			ini.option("format", "group", &config.format.column[Column::GROUP]);
		}
		if (ini.has_option("format", "level")) {
			ini.option("format", "level", &config.format.column[Column::LEVEL]);
		}
		config.format.update_fieldset();

		//buff
		if (ini.has_option("buff", "enable")) {
			ini.option_num("buff", "enable", &config.buff.enable);
		}
		if (ini.has_option("buff", "size")) {
			ini.option_num("buff", "size", &config.buff.size);
		}
		if (ini.has_option("buff", "enable_line_buff")) {
			ini.option_num("buff", "enable_line_buff", &config.buff.enable_line_buff);
		}
		if (ini.has_option("buff", "line_buff_size")) {
			ini.option_num("buff", "line_buff_size", &config.buff.line_buff_size);
		}
		if (ini.has_option("buff", "enable_msg_buff")) {
			ini.option_num("buff", "enable_msg_buff", &config.buff.enable_msg_buff);
		}
		if (ini.has_option("buff", "msg_max_size")) {
			ini.option_num("buff", "msg_max_size", &config.buff.msg_max_size);
		}

		//output
		if (ini.has_option("output", "file_name")) {
			ini.option("output", "file_name", &config.output.file_name);
		}
		if (ini.has_option("output", "backup_folder")) {
			ini.option("output", "backup_folder", &config.output.backup_folder);
		}
		if (ini.has_option("output", "backup_period")) {
			ini.option_num("output", "backup_period", &config.output.backup_period);
		}

		return 0;
	}
}
