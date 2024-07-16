#pragma once
#include <string>
#include <vector>
#include "open_da_property.h"
#include "SimpleIni.h"
#include "date_time.h"

struct config
{
	//! Constructor to load the config file into memory.
	/*
	\param[in] file_name: name of the config file to be loaded into memory
	*/
	config(std::string file_path, std::string const& config_file);
	std::string wanda_bin; //path to the wanda binary
	std::string file_path; //path to the executable
	std::string wanda_case; //name to the wanda case
	std::vector<open_da_property> properties; //vector of open_da_property objects, which need to be set in the model
	date_time start_time; //start time of the simulation
	date_time end_time; //end time of the simulation
	int simulation_time_step; //time step of the simulation
	//!Function to parse the config file and load the properties into memory.
	void parse_properties(CSimpleIniA const& config_data);
	//!Function to parse the general data of the config file and load it into memory.
	void parse_general_data(CSimpleIniA const& config_data);
	std::string get_model_path() const { return file_path + "\\" + wanda_case; }
};

