#pragma once
#include <string>
#include <vector>
#include "date_time.h"

class csv_file
{
private:
	std::string file_name;
	char delimiter;
	std::vector<date_time> dates;
	std::vector<float> data_value;
	void read_csv();
public:
	csv_file(std::string file_name, char delimiter = ',');
	csv_file(){}
	//! Get the data time and value from the csv file
	std::vector<date_time> get_data_time() const { return dates; }
	//! Get the data value from the csv file
	std::vector<float> get_data_value() const { return data_value; }
	//! Get the time series between start and end time
	//! @param time_start The start time for which the data should be returned
	//! @param time_end The end time for which the data should be returned
	//! @return A vector of vectors with the first vector containing the time starting at zero at the starttime
	//and the second vector containing the data value
	std::vector<std::vector<float>> get_time_series(date_time time_start, date_time time_end) const;
};

//! Split a string into a vector of strings based on the given delimiter
//! @param input_string The string to be split
//! @param delimiter The delimiter to split the string on default is '.'
//! @return A vector of string splitted into strings.
std::vector<std::string> split_string(std::string input_string, char delimiter = '.');