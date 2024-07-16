#include "wanda.h"
#include <filesystem>
#include <stdexcept>
#include "date_time.h"

void prepare_wanda_model(config config_data, wanda_model& model)
{
	//setting global parameters
	model.get_property("Simulation time").set_scalar(float(config_data.end_time.duration(config_data.start_time)));
	model.get_property("Time step").set_scalar(float(config_data.simulation_time_step));

	//setting parameters
	for (auto& property : config_data.properties)
	{
		auto& comp = model.get_component(property.get_comp_name());
		auto& wanda_property = comp.get_property(property.get_property_name());
		if (property.has_table_description())
		{
			set_wanda_property_table(wanda_property, property);
		}
		else if (property.get_property_name().find("table") != std::string::npos)
		{
			set_action_table(wanda_property, property, config_data.start_time, config_data.end_time );
			//Temporary solution to set the initial value.
			//It is assumed that only the mass flow rate can be set.
			// It would be nicer if we via the action table could access the related property and set it.
			if (property.get_property_name() == "Action table")
			{
				auto value = property.get_time_series(config_data.start_time, config_data.end_time)[1][0];
				comp.get_property("Mass flow at t = 0 [s]").set_scalar(value);

			}
		}
		else
		{
			wanda_property.set_scalar(property.get_value());
		}
	}
	//setting start time for seawat coupling filter pipes in WANDA
	set_start_time_seawat(model, config_data.start_time.to_string());
	model.save_model_input();
}

void set_start_time_seawat(wanda_model& model, std::string start_time)
{
	for(const auto comp: model.get_all_components())
	{
		if (comp->get_type_name() != "Heat filter pipe geoformation")
		{
			continue;
		}
		auto& table = comp->get_property("Time of SEAWAT resume").get_table();
		std::vector<std::string> start_time_vector;
		start_time_vector.push_back(start_time);
		table.set_string_column("Time", start_time_vector);
		// check if restart file exists
		auto case_path = model.get_case_path();
		//This assumes there is only one heat filter pipe in the model (which stores its seawat results in the HTO_001 folder)
		std::string file_name = case_path.substr(0, case_path.size() - 4) + "\\HTO_001\\HTO_TEMP_" + start_time + ".ASC";
		//if the file exists, the model will started from this file, otherwise it will be restarted from the beginning.
		bool const restart = std::filesystem::exists(file_name);
		comp->get_property("SEAWAT resume").set_scalar(restart? float(2.0):float(1.0));
	}	
}

void run_wanda_model(wanda_model& model)
{
	//TODO add checks to see if steady or unsteady give errors
	model.run_steady();	
	model.run_unsteady();
}

void set_wanda_property_table(wanda_property& wanda_prop, open_da_property prop)
{
	auto& table = wanda_prop.get_table();
	auto names = table.get_string_column("Name of geoformation");	
	auto ind = std::find(names.begin(), names.end(), prop.get_table_row_identifier());
	
	if (ind == names.end())
	{
		throw std::runtime_error(prop.get_table_row_identifier() + " not in table");
	}
	auto const index = std::distance(names.begin(), ind);
	auto col_values = table.get_float_column(prop.get_table_column_description());
	col_values.at(index) = prop.get_value();
	table.set_float_column(prop.get_table_column_description(), col_values);
}

void set_action_table(wanda_property& wanda_prop, open_da_property const& prop, date_time const& start_time, date_time const& end_time)
{
	auto& table = wanda_prop.get_table();	
	table.set_float_data(prop.get_time_series(start_time, end_time));

}

