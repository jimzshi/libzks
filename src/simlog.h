#ifndef _ZKS_SIMLOG
#define _ZKS_SIMLOG

#include "u8string.h"

#include <array>
#include <mutex>
#include <iosfwd>
#include <chrono>
#include <ctime>
#include <memory>
#include <thread>

namespace zks {

	class simlog {
	public:
		enum Level { 
			EMERG, FATAL, ALERT, CRIT, 
			ERROR, WARN, NOTICE, INFO, 
			DEBUG, TRACE, 
			lsize = 10 
		};
		static const std::array<u8string, Level::lsize> level_string;

		enum Column { 
			DATETIME, EPOCHTIME, THREAD, FILE, 
			FUNC, LINE, GROUP, LEVEL, MESSAGE,
			csize = 9
		};
		static const std::array<u8string, Column::csize> column_string;
		
		struct Config {
			struct Format {
				//u8string locale;
				u8string strftime;
				u8string seperator;
				u8string quote;
				u8string escape;
				std::array<u8string, Column::csize> column;
				std::bitset<Column::csize> fieldset;

				Format() : 
					//locale(""), 
					strftime("%x-%X"), seperator(","), quote("\""), escape("\\") {
					column[Column::DATETIME] = "%s";
					column[Column::EPOCHTIME] = "%8x";
					column[Column::THREAD] = "|%8X|";
					column[Column::FILE] = "%s";
					column[Column::FUNC] = ":%s:";
					column[Column::LINE] = "(%d)";
					column[Column::GROUP] = "<%s>";
					column[Column::LEVEL] = "[%s]";
					column[Column::MESSAGE] = "%s";
					fieldset.set();
				}
				~Format() {}
				void update_fieldset() {
					fieldset.set();
					for(size_t i = 0; i<column.size(); ++i) {
						if(column[i].empty()) {
							if(i == Column::EPOCHTIME || i==Column::THREAD || i== Column::LINE) {
								column[i] = "%d";
							} else {
								column[i] = "%s";
							}
							fieldset.reset(i);
						}
					}
				}
			};
			struct Buff {
				bool enable;
				size_t size;
				bool enable_line_buff;
				size_t line_buff_size;
				bool enable_msg_buff;
				size_t msg_max_size;
				Buff() : enable(true), size(1000000),
					enable_line_buff(true), line_buff_size(1512), 
					enable_msg_buff(true), msg_max_size(1000) {}
				~Buff() {}
			};
			struct Output {
				u8string file_name;
				int backup_period;
				u8string backup_folder;
				Output() : file_name("log.csv"), backup_period(0), backup_folder(".") {}
				~Output() {}
			};
		
		public:
			int stream_type;
			int thread_safe;
			int verbosity;
			Format format;
			Buff buff;
			Output output;
			Config() : stream_type(2), thread_safe(1), verbosity(5), format(), buff(), output() {}
			~Config() {}
		};

	private:
		typedef std::recursive_mutex mutex_t;
		typedef std::lock_guard<mutex_t> lock_guard_t;
		typedef std::chrono::system_clock clock_t;

		std::hash<std::thread::id> thread_hash_;
		std::shared_ptr<std::ostream> ostream_;
		u8string fmt_str_;
		
		u8string line_buff_;
		u8string msg_buff_;
		u8string buff_;
		clock_t::time_point epoch_;

	public:
		Config config;
		mutex_t mutex;

	public:
		simlog() { reset(); }
		simlog(u8string const& ini_file);
		simlog(int mode, u8string const& file_name = "");
		simlog(simlog&& rhs) {
			this->flush();
			rhs.flush();
			lock_guard_t Here(rhs.mutex);
			config = std::move(rhs.config);
			epoch_ = std::move(rhs.epoch_);
			fmt_str_ = std::move(rhs.fmt_str_);
			ostream_ = std::move(rhs.ostream_);
		}
		~simlog() {
			if (!buff_.is_null() && ostream_->good()) {
				flush();
			}
		}

		void reset() {
			flush();
			reset_epoch_();
			reset_ostream_();
			reset_buff_();
			generate_fmtstr_();
		}
		simlog& flush() {
			lock_guard_t here(mutex);
			if (ostream_) {
				(*ostream_) << buff_;
				buff_.clear();
				ostream_->flush();
			}
			return *this;
		}
		void write(const char* file, const char* func, int line, u8string const& group, Level level, const char* fmt, ...);
		int backup();
		int configure(u8string const& ini_file);

		simlog& operator<<(simlog& (*func)(simlog&)) {
			return func(*this);
		}
		template<typename T_>
		simlog& operator<<(T_ const& c) {
			lock_guard_t here(mutex);
			if (!buff_.is_null()) {
				*ostream_ << buff_;
				buff_.clear();
			}
			*ostream_ << c;
			return *this;
		}

	private:
		void generate_fmtstr_() {
			fmt_str_ = config.format.seperator.join(config.format.column.begin(), config.format.column.end(), config.format.quote, config.format.escape) + "\n";
		}
		void reset_buff_();
		void reset_epoch_() {
			epoch_ = clock_t::now();
		}
		void reset_ostream_();
		u8string get_datetime_();
		
		//class locale_guard{
		//	std::locale old_;
		//public:
		//	locale_guard(std::locale l) {
		//		old_ = std::locale::global(l);
		//	}
		//	~locale_guard() {
		//		std::locale::global(old_);
		//	}
		//};
	};

	inline
	simlog& endl(simlog& logger) {
		logger << "\n";
		return logger.flush();
	}

#define ZKS_TRACE(logger, group, fmt, ...) logger.write(__FILE__, __func__, __LINE__, group, zks::simlog::Level::TRACE, fmt, __VA_ARGS__)
#define ZKS_DEBUG(logger, group, fmt, ...) logger.write(__FILE__, __func__, __LINE__, group, zks::simlog::Level::DEBUG, fmt, __VA_ARGS__)
#define ZKS_INFO(logger, group, fmt, ...) logger.write(__FILE__, __func__, __LINE__, group, zks::simlog::Level::INFO, fmt, __VA_ARGS__)
#define ZKS_NOTICE(logger, group, fmt, ...) logger.write(__FILE__, __func__, __LINE__, group, zks::simlog::Level::NOTICE, fmt, __VA_ARGS__)
#define ZKS_WARN(logger, group, fmt, ...) logger.write(__FILE__, __func__, __LINE__, group, zks::simlog::Level::WARN, fmt, __VA_ARGS__)
#define ZKS_ERROR(logger, group, fmt, ...) logger.write(__FILE__, __func__, __LINE__, group, zks::simlog::Level::ERROR, fmt, __VA_ARGS__)
#define ZKS_CRIT(logger, group, fmt, ...) logger.write(__FILE__, __func__, __LINE__, group, zks::simlog::Level::CRIT, fmt, __VA_ARGS__)
#define ZKS_ALERT(logger, group, fmt, ...) logger.write(__FILE__, __func__, __LINE__, group, zks::simlog::Level::ALERT, fmt, __VA_ARGS__)
#define ZKS_FATAL(logger, group, fmt, ...) logger.write(__FILE__, __func__, __LINE__, group, zks::simlog::Level::FATAL, fmt, __VA_ARGS__)
#define ZKS_EMERG(logger, group, fmt, ...) logger.write(__FILE__, __func__, __LINE__, group, zks::simlog::Level::EMERG, fmt, __VA_ARGS__)

}

#endif
