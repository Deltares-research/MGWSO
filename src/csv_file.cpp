#include <fstream>
#include <iostream>
#include <string>
#include "csv_file.h"

#include <filesystem>


csv_file::csv_file(std::string file_name, char delimiter) : 
	file_name(file_name), delimiter(delimiter)
{
	if (not std::filesystem::exists(file_name))
	{
		throw std::runtime_error("File does not exist");
	}
	read_csv();
}

void csv_file::read_csv()
{
	std::ifstream file(file_name);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file");
	}
	std::string line;
	while (std::getline(file, line))
	{
		auto result = split_string(line, delimiter);
		date_time dt(result[0]);
		dates.push_back(dt);
		data_value.push_back(std::stof(result[1]));
	}
}

std::vector<std::vector<float>> csv_file::get_time_series(date_time time_start, date_time time_end) const
{
    if (time_start > time_end)
    {
        throw std::runtime_error("Start time is greater than end time");
    }
    if(time_start > dates.back() || time_end < dates.front())
    {
        throw std::runtime_error("Time range is outside of data range");
    }
    std::vector<float> res;
    std::vector<float> time;
    for (size_t i = 0; i < dates.size(); i++)
    {
	    if (dates[i] >= time_start && dates[i] <= time_end)
	    {
		    res.push_back(data_value[i]);
		    time.push_back(float(dates[i].duration(time_start)));
	    }
    }
    std::vector<std::vector<float>> results;
    results.push_back(time);
    results.push_back(res);
    return results;
}