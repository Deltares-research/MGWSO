#include "config.h"
#include <spdlog/spdlog.h>
#include <iostream>

#include "date_time.h"
#include "utility.h"


config::config(std::string _file_path, std::string const& config_file)
{
	auto splitted_path = split_string(_file_path, '\\');
	file_path = join_string(splitted_path.begin(), splitted_path.end() - 1, '\\');
	CSimpleIniA config_file_data;
	config_file_data.SetUnicode();
	spdlog::debug(file_path + "\\" + config_file);
	SI_Error rc = config_file_data.LoadFile((file_path + "\\" +  config_file).c_str());
	//TODO add error handling
	spdlog::debug(rc);
	spdlog::debug( "Parsing general data");
	parse_general_data(config_file_data);
	spdlog::debug("Parsing properties");
	parse_properties(config_file_data);
	spdlog::debug("Finished parsing properties");
}

void config::parse_properties(CSimpleIniA const& config_data)
{
	CSimpleIniA::TNamesDepend keys;
	std::string const parameter_key = "parameters";
	config_data.GetAllKeys(parameter_key.c_str(), keys);
	for (const auto& element :keys )
	{
		spdlog::debug(element.pItem);
		auto splitted_string = split_string(element.pItem, '.');
		auto value_string = std::string(config_data.GetValue(parameter_key.c_str(), element.pItem));		
		if(splitted_string.size() == 4)
		{
			properties.emplace_back(open_da_property(splitted_string[0], splitted_string[1], 
				splitted_string[2], splitted_string[3], std::stof(value_string)));
		}
		else if (splitted_string.size() == 2) {
			if (value_string.substr(value_string.size() - 3, 3) == "csv")
			{
				spdlog::debug("csv file found");
				spdlog::debug(splitted_string[0]);
				spdlog::debug(splitted_string[1]);
				spdlog::debug(value_string);
				properties.emplace_back(open_da_property(splitted_string[0], splitted_string[1], value_string));
				spdlog::debug("csv file added");
			}
			else
			{
				spdlog::debug("csv file not found");
				properties.emplace_back(open_da_property(splitted_string[0], splitted_string[1], std::stof(value_string)));
			}
		}
		else
		{
			throw std::runtime_error("Unsupported number of elements in parameter list of ini file");
		}
	}
}

void config::parse_general_data(CSimpleIniA const& config_data)
{
	std::string const general_key = "run";
	start_time = date_time(config_data.GetValue(general_key.c_str(), "startDateTime"));
	end_time = date_time(config_data.GetValue(general_key.c_str(), "endDateTime"));
	simulation_time_step = std::stoi(config_data.GetValue(general_key.c_str(), "timeStep"));
	wanda_bin = config_data.GetValue(general_key.c_str(), "wandaBin");
	wanda_case = config_data.GetValue(general_key.c_str(), "wandaModel");
}
