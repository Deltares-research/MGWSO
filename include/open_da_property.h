#pragma once
#include <string>
#include "csv_file.h"
#include <iostream>

class open_da_property
	{
public:
	open_da_property(std::string comp_name, std::string property_name, float value) :
		_comp_name(comp_name), _property_name(property_name), _value(value), _table_row_identifier(""),
		_table_column_description(""), _csv_file("")
	{};
	open_da_property(std::string comp_name, std::string property_name, std::string csv_file_in) :
		_comp_name(comp_name), _property_name(property_name), _value(0.0), _table_row_identifier(""),
		_table_column_description(""), _csv_file(csv_file_in)
	{
		_csv_data = csv_file(_csv_file, ';');
	};
	open_da_property(std::string comp_name, std::string property_name, std::string table_column_description,
		std::string table_row_identifier, float value) :
		_comp_name(comp_name),  _property_name(property_name), _table_column_description(table_column_description),
		_table_row_identifier(table_row_identifier), _value(value), _csv_file("")
	{		
	};
	std::string get_comp_name() { return _comp_name;}
	std::string get_property_name() const { return _property_name; }
	std::string get_table_column_description() { return _table_column_description; }
	std::string get_table_row_identifier() { return _table_row_identifier;}
	float get_value() const { return _value; }
	bool has_table_description() const { return !_table_column_description.empty();}
	std::vector<std::vector<float>> get_time_series(date_time time_start, date_time time_end) const
	{
		return _csv_data.get_time_series(time_start, time_end);
	}
private:
	std::string _comp_name;
	std::string _property_name;
	std::string _table_column_description;
	std::string _table_row_identifier;
	std::string _csv_file;
	csv_file _csv_data;
	float _value;
};

