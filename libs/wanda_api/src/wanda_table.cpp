#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <wanda_table.h>

std::string wanda_table::_object_name = "WandaTable Object";

wanda_table::wanda_table() : _object_hash(std::hash<std::string>{}(_object_name))
{
}

void wanda_table::clear_table()
{
    for (auto &item : table_data)
    {
        item.second.floattable.clear();
        item.second.stringtable.clear();
    }
    set_modified(true);
}

std::vector<std::vector<float>> wanda_table::get_float_data()
{
    std::vector<std::vector<float>> values;
    for (auto description : _description)
    {
        if (table_data[description]._table_type != 'S')
        {
            values.push_back(table_data[description].floattable);
        }
    }
    return values;
}

void wanda_table::set_float_data(std::vector<std::vector<float>> values)
{
    // TODO add check on size of values
    int index = 0;
    for (auto description : _description)
    {
        if (table_data[description]._table_type != 'S')
        {
            table_data[description].floattable = values[index];
            index++;
        }
    }
    _is_modified = true;
}

std::vector<std::vector<std::string>> wanda_table::get_string_data()
{
    std::vector<std::vector<std::string>> values;
    for (auto description : _description)
    {
        if (table_data[description]._table_type == 'S')
        {
            values.push_back(table_data[description].stringtable);
        }
    }

    return values;
}

void wanda_table::set_string_data(std::vector<std::vector<std::string>> values)
{
    // TODO add check on size of values
    int index = 0;
    for (auto description : _description)
    {
        if (table_data[description]._table_type == 'S')
        {
            table_data[description].stringtable = values[index];
            index++;
        }
    }
    _is_modified = true;
}

std::vector<std::string> wanda_table::get_descriptions() const
{
    return _description;
}

std::vector<char> wanda_table::get_table_types()
{
    std::vector<char> types;
    for (auto description : _description)
    {
        if (table_data[description]._table_type == 'S')
        {
            types.push_back('S');
        }
        else
        {
            types.push_back('F');
        }
    }
    return types;
}

std::string wanda_table::get_key(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        return table_data[description]._key;
    }
    throw std::invalid_argument(description + " not in table");
}

void wanda_table::set_key(std::string description, std::string key)
{
    if (table_data.find(description) != table_data.end())
    {
        table_data[description]._key = key;
        return;
    }
    throw std::invalid_argument(description + " not in table");
}

void wanda_table::set_keys(const std::string key)
{
    for (const auto descr : _description)
    {
        set_key(descr, key);
    }
}

char wanda_table::get_spec_code(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        return table_data[description]._spec_code;
    }
    throw std::invalid_argument(description + " not in table");
}

void wanda_table::add_column(std::string description, std::string unit, std::string key, char type, int index,
                             int col_num, std::string related_descr, char spec_code)
{
    if (table_data.find(description) == table_data.end())
    {
        _description.push_back(description);
        wanda_table_data tab_data;
        tab_data._index = index;
        tab_data._key = key;
        tab_data._table_type = type;
        tab_data._col_num = col_num;
        if (!related_descr.empty())
        {
            tab_data._related_description.push_back(related_descr);
        }
        tab_data._unit = unit;
        tab_data._spec_code = spec_code;
        table_data.emplace(description, tab_data);
        return;
    }
    // if the description is already in the table and it is a new table then they should merge
    if (type == 'T' && col_num == 0)
    {
        table_data[description]._related_description.push_back(related_descr);
        table_data[description]._key = key;
        return;
    }
    if (type == 'T' && col_num == 1)
    {
        auto descr = description + std::to_string(col_num);
        _description.push_back(descr);
        wanda_table_data tab_data;
        tab_data._index = index;
        tab_data._key = key;
        tab_data._table_type = type;
        tab_data._col_num = col_num;
        tab_data._unit = unit;
        tab_data._spec_code = spec_code;
        if (!related_descr.empty())
        {
            tab_data._related_description.push_back(related_descr);
        }
        table_data.emplace(descr, tab_data);
        return;
    }
    if (table_data[description]._index == index)
    {
        // column is already in table and should not be added again
        return;
    }
    // name of the column already exist thus add column number to it.
    auto descr = description + std::to_string(index - table_data[related_descr]._index);
    _description.push_back(descr);
    wanda_table_data tab_data;
    tab_data._index = index;
    tab_data._key = key;
    tab_data._table_type = type;
    tab_data._col_num = col_num;
    tab_data._unit = unit;
    tab_data._spec_code = spec_code;
    table_data.emplace(descr, tab_data);
}

void wanda_table::set_float_column(std::string description, std::vector<float> values)
{
    if (table_data.find(description) != table_data.end())
    {
        if (table_data[description]._table_type == 'S')
        {
            throw std::runtime_error(description + " is not a float column");
        }
        table_data[description].floattable = values;
        set_modified(true);
        return;
    }
    throw std::invalid_argument(description + " not in table");
}

void wanda_table::set_string_column(std::string description, std::vector<std::string> values)
{
    if (table_data.find(description) != table_data.end())
    {
        if (table_data[description]._table_type != 'S')
        {
            throw std::runtime_error(description + " is not a string collumn");
        }
        table_data[description].stringtable = values;
        set_modified(true);
        return;
    }
    throw std::invalid_argument(description + " not in table");
}

std::vector<float> wanda_table::get_float_column(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        if (table_data[description]._table_type == 'S')
        {
            throw std::runtime_error(description + " is not a float collumn");
        }
        return table_data[description].floattable;
    }
    throw std::invalid_argument(description + " not in table");
}

float *wanda_table::get_float_column_pnt(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        if (table_data[description]._table_type == 'S')
        {
            throw std::runtime_error(description + " is not a float column");
        }
        if (table_data[description].floattable.size())
        {
            return table_data[description].floattable.data();
        }
        throw std::runtime_error(description + " has no data");
    }
    throw std::invalid_argument(description + " not in table");
}

std::vector<std::string> wanda_table::get_string_column(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        if (table_data[description]._table_type != 'S')
        {
            throw std::runtime_error(description + " is not a string collumn");
        }
        return table_data[description].stringtable;
    }
    throw std::invalid_argument(description + " not in table");
}

int wanda_table::get_index(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        return table_data[description]._index;
    }
    throw std::invalid_argument(description + " not in table");
}

char wanda_table::get_table_type(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        return table_data[description]._table_type;
    }
    throw std::invalid_argument(description + " not in table");
}

int wanda_table::get_col_num(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        return table_data[description]._col_num;
    }
    throw std::invalid_argument(description + " not in table");
}

bool wanda_table::has_data()
{
    for (auto description : get_descriptions())
    {
        if (has_data(description))
        {
            return true;
        }
    }
    return false;
}

bool wanda_table::has_data(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        const auto tab_data = &table_data[description];
        if (tab_data->_table_type == 'S')
        {
            return !tab_data->stringtable.empty();
        }
        return !tab_data->floattable.empty();
    }
    throw std::invalid_argument(description + " not in table");
}

std::vector<std::string> wanda_table::get_related_prop(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        return table_data[description]._related_description;
    }
    throw std::invalid_argument(description + " not in table");
}

std::string wanda_table::get_unit(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        return table_data[description]._unit;
    }
    throw std::invalid_argument(description + " not in table");
}

bool wanda_table::has_description(std::string description)
{
    return table_data.find(description) != table_data.end();
}

bool wanda_table::is_string_column(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        return table_data[description]._table_type == 'S';
    }
    throw std::invalid_argument(description + " not in table");
}

wanda_table_data *wanda_table::get_table_data(std::string description)
{
    if (table_data.find(description) != table_data.end())
    {
        return &table_data[description];
    }
    throw std::invalid_argument(description + " not in table");
}

bool wanda_table::check_table()
{
    auto &first_column = table_data.at(_description[0]);
    auto size = first_column.get_size();
    if (size == 0)
    {
        return false; // Table data is not initialized?
    }
    // check if size of all columns is the same
    bool table_status =
        std::all_of(table_data.begin(), table_data.end(), [&](auto &i) { return i.second.get_size() == size; });
    if (first_column._table_type == 'T')
    {
        // if table then check if the first column is increasing.
        return table_status && std::is_sorted(first_column.floattable.begin(), first_column.floattable.end());
    }
    return table_status;
}

void wanda_table::copy_table(wanda_table table_org)
{
    resize_columns(table_org.get_max_column_size());
    for (auto &description : get_descriptions())
    {
        if (table_org.has_description(description))
        {
            if (!table_org.has_data(description))
                continue; // don't copy empty columns

            if (is_string_column(description))
            {
                set_string_column(description, table_org.get_string_column(description));
                continue;
            }
            set_float_column(description, table_org.get_float_column(description));
        }
    }
}

void wanda_table::resize_columns(std::size_t size)
{
    // Resizing to 0 rows is invalid,  destroys all data
    if (size > 0)
    {
        for (auto &[key, val] : table_data)
            val.resize(size);
    }
}

std::size_t wanda_table::get_max_column_size() const
{
    const auto result = std::max_element(table_data.cbegin(), table_data.cend(), [](const auto &a, const auto &b) {
        return a.second.get_size() < b.second.get_size();
    });
    return result->second.get_size();
}

void wanda_table::resize_table_to_max_column_size()
{    
    resize_columns(get_max_column_size());
}
