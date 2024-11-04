#pragma once
#include "config.h"
#include "wandamodel.h"


//!Function which sets the data in the wanda model.
//!@param config_data The configuration data to set in the Wanda model.
//!@param model The Wanda model to set the configuration data in.
void prepare_wanda_model(config config_data, wanda_model& model);

//!function to run the Wanda model. First a steady simulation, followed by a transient simulation.
//!@param model The Wanda model to run.
void run_wanda_model(wanda_model& model);

//!Function to set the properties in tables of the Wanda model.
//For now the function only sets the properties in the table of the Heat filter pipe geoformation.
//!@param wanda_prop The property in the Wanda model to be set.
//!@param prop The property in the open_da_property class used to set the wanda property.
void set_wanda_property_table(wanda_property& wanda_prop, open_da_property prop);

//!Function to set the start and end time for seawat in the Wanda model.
//!@param model The Wanda model to set the start time for the seawat filter components.
//!@param start_time The start time to set in the Wanda model.
void set_start_time_seawat(wanda_model& model, std::string start_time);

//!Function to set the action table in the Wanda model.
//!@param wanda_prop The property in the Wanda model to be set.
//!@param prop The property in the open_da_property class used to set the wanda property.
//!@param start_time The start time of the simulation.
//!@param end_time The end time of the simulation.
//!@param time_step The time step of the simulation.
void set_action_table(wanda_property& wanda_prop, open_da_property const& prop, date_time const& start_time, date_time const& end_time);

//!Function to delete the output file of the Wanda model.
//!@param model_path The path to the model file.
void delete_output_file(std::string const& model_path);
