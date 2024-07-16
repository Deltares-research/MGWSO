#include "date_time.h"
using namespace std::chrono_literals;

date_time::date_time(std::string datetimeString, std::string format)

{
	tm tmStruct = {};
	std::istringstream ss(datetimeString);
	ss >> std::get_time(&tmStruct, format.c_str());
	_time = std::chrono::system_clock::from_time_t(
		mktime(&tmStruct));
	_data_time_string = datetimeString;
}

long long date_time::duration(const date_time& time) const
{
	return std::chrono::duration_cast<std::chrono::seconds>(_time - time._time).count();
}


