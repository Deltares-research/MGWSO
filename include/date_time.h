#pragma once
#include <chrono>
class date_time
{
private:
	std::chrono::system_clock::time_point _time;
	std::string _data_time_string;
public:
	date_time(std::string datetimeString, std::string format = "%Y%m%d%H%M%S");
	date_time() {}
	long long duration(const date_time& time) const;
	std::string to_string() { return _data_time_string; }
	bool operator >(const date_time& time) const {return duration(time) > 0; }
	bool operator ==(const date_time& time) const {return time._time == _time;}
	bool operator !=(const date_time& time) const {return !(*this == time); }
	bool operator <(const date_time& time) const { return !(*this > time || *this == time); }
	bool operator >=(const date_time& time) const { return !(*this < time); }
	bool operator <=(const date_time& time) const { return !(*this > time); }
};

