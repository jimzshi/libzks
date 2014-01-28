#ifndef _ZKS_STOPWATCH
#define _ZKS_STOPWATCH

#include "u8string.h"

#include <chrono>
#include <vector>

namespace zks {

	/*
	 *	Time Unit is milli-second (ms)
	 */
	class StopWatch {
	public:
		using time_unit = std::chrono::milliseconds;
		using clock_type = std::chrono::high_resolution_clock;
		using time_point = clock_type::time_point;
		struct TimePoint {
			time_point tp;
			u8string job;
			time_unit duration;
		};
		

		StopWatch() {}
		StopWatch(const StopWatch&) = delete;
		const StopWatch& operator=(const StopWatch&) = delete;
		~StopWatch() {}

		time_unit total() const {
			return std::chrono::duration_cast<time_unit>(_tpv_[size()].tp - _tpv_[0].tp);
		}
		time_unit duration(size_t pos) const {
			return (pos==0) ? time_unit() : _tpv_[pos].duration;
		}
		TimePoint const& operator[](size_t p) const {
			return _tpv_[p];
		}
		size_t size() const {
			return _tpv_.size() - 1;  // 0 is start time point;
		}
		u8string u8str() const {//json
			u8string buff{ R"(zks::StopWatch##{"Title":)" };
			buff.append(_tpv_[0].job.quote() + R"(, "jobs":[)");
			size_t sz = size();
			for (size_t i = 1; i <= sz; ++i) {
				buff += "{" + _tpv_[i].job.quote() + ":" + to_u8string(_tpv_[i].duration.count()) + "}";
				if (i < sz) {
					buff += ", ";
				}
			}
			buff += "]}";
			return buff;
		}

		void start(u8string const& cmt = "") {
			reset();
			TimePoint tmp;
			tmp.tp = clock_type::now();
			tmp.job = cmt;
			_tpv_.push_back(std::move(tmp));
		}
		void reset() {
			_tpv_.clear();
		}
		time_unit tick(u8string const& cmt = "") {
			TimePoint tmp;
			tmp.tp = clock_type::now();
			tmp.job = cmt;
			tmp.duration = std::chrono::duration_cast<time_unit>(tmp.tp - _tpv_.back().tp);
			if (tmp.job.is_null()) {
				tmp.job.format(7, "job %d", _tpv_.size());
			}
			_tpv_.push_back(std::move(tmp));
			return duration(size());
		}

	private:
		std::vector<TimePoint> _tpv_;
	};

	inline std::ostream& operator<<(std::ostream& os, const StopWatch& sw) {
		os << sw.u8str();
		return os;
	}

} /* namespace zks */


#endif