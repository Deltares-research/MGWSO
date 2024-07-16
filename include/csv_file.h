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
	std::vector<date_time> get_data_time() const { return dates; }
	std::vector<float> get_data_value() const { return data_value; }
	std::vector<std::vector<float>> get_time_series(date_time time_start, date_time time_end) const;
};

std::vector<std::string> split_string(std::string input_string, char delimiter = '.');