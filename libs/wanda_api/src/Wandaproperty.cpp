#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>
#include <wandaproperty.h>

std::string wanda_property::_object_name = "WandaProperty Object";

wanda_property::wanda_property()
    : _wnd_type(wanda_property_types::NONE), _description("not available"), _index(0), _group_index(0),
      _comp_spec_code(' '), _comp_sp_inp_fld(' '), _wdo_post_fix("-"), _unit_dim("."), _modified(false),
      _scalar(-999.0), _default_value(0), _min_value(0), _max_value(0), _series(nullptr), _number_of_elements(0),
      _object_hash(std::hash<std::string>{}(_object_name))
{
    extr_min.push_back(nullptr);
    extr_max.push_back(nullptr);
    extr_tmin.push_back(nullptr);
    extr_tmax.push_back(nullptr);
}

wanda_property::wanda_property(int index, std::string spec_descr, char comp_spec_code, char comp_sp_inp_fld,
                               std::string wdo_post_fix, std::string unit_dim, wanda_property_types wnd_type,
                               std::string short_quant_name, float def_val, float min_val, float max_val,
                               std::string list_dependency, int view_list_mask, char input_type_code, int view_mask)
    : _modified(false), _scalar(def_val), _series(nullptr), _wnd_type(wnd_type), _description(spec_descr),
      _index(index), _group_index(0), _comp_spec_code(comp_spec_code), _comp_sp_inp_fld(comp_sp_inp_fld),
      _wdo_post_fix(wdo_post_fix), _unit_dim(unit_dim), _default_value(def_val), _min_value(min_val),
      _max_value(max_val), _number_of_elements(0), _short_quant_name(short_quant_name),
      _list_dependency(list_dependency), _view_list_mask(view_list_mask), _input_type_code(input_type_code),
      _view_mask(view_mask), _object_hash(std::hash<std::string>{}(_object_name))
{
    extr_min.push_back(nullptr);
    extr_max.push_back(nullptr);
    extr_tmin.push_back(nullptr);
    extr_tmax.push_back(nullptr);
    _series_pipe.push_back(nullptr);
    _spec_status = _default_value != -999;
    if (_comp_sp_inp_fld == 'C')
    {
        _spec_status = _default_value != 0;
    }
}

void wanda_property::set_unit_factor(std::unordered_map<std::string, std::unordered_map<std::string, float>> &unit_list,
                                     std::unordered_map<std::string, std::string> &case_units)
{
    auto unit_dim = get_unit_dim();
    auto it1 = unit_list.find(unit_dim);
    auto it2 = case_units.find(unit_dim);
    if (case_units.find(unit_dim) != case_units.end())
    {
        set_unit_factor(unit_list[unit_dim][case_units[unit_dim]]);
        return;
    }
    set_unit_factor(1.0);
}

void wanda_property::set_extremes(float *min, float *max, float *Tmin, float *Tmax)
{
    extr_min[0] = min;
    extr_max[0] = max;
    extr_tmin[0] = Tmin;
    extr_tmax[0] = Tmax;
}

void wanda_property::set_extremes(int element, float *min, float *max, float *Tmin, float *Tmax)
{
    if (element <= _number_of_elements)
    {
        extr_min[element] = min;
        extr_max[element] = max;
        extr_tmin[element] = Tmin;
        extr_tmax[element] = Tmax;
        return;
    }
    throw std::runtime_error("Element not within total number of elements");
}

float wanda_property::get_extr_min() const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    if (extr_min[0] == nullptr)
    {
        throw std::runtime_error("Data not loaded");
    }
    return *extr_min[0];
}

float wanda_property::get_extr_max() const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    if (extr_max[0] == nullptr)
    {
        throw std::runtime_error("Data not loaded");
    }
    return *extr_max[0];
}

float wanda_property::get_extr_tmin() const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    if (extr_tmin[0] == nullptr)
    {
        throw std::runtime_error("Data not loaded");
    }
    return *extr_tmin[0];
}

float wanda_property::get_extr_tmax() const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    if (extr_tmax[0] == nullptr)
    {
        throw std::runtime_error("Data not loaded");
    }
    return *extr_tmax[0];
}

float wanda_property::get_extr_min(int element) const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    if (extr_min[element] == nullptr)
    {
        throw std::runtime_error("Data not loaded");
    }
    return *extr_min[element];
}

float wanda_property::get_extr_max(int element) const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    if (extr_max[element] == nullptr)
    {
        throw std::runtime_error("Data not loaded");
    }
    return *extr_max[element];
}

float wanda_property::get_extr_tmin(int element) const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    if (extr_tmin[element] == nullptr)
    {
        throw std::runtime_error("Data not loaded");
    }
    return *extr_tmin[element];
}

float wanda_property::get_extr_tmax(int element) const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    if (extr_tmax[element] == nullptr)
    {
        throw std::runtime_error("Data not loaded");
    }
    return *extr_tmax[element];
}

std::vector<float> wanda_property::get_extr_min_pipe() const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    std::vector<float> result;
    for (int i = 0; i <= _number_of_elements; i++)
    {
        result.push_back(get_extr_min(i));
    }
    return result;
}

std::vector<float> wanda_property::get_extr_max_pipe() const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    std::vector<float> result;
    for (int i = 0; i <= _number_of_elements; i++)
    {
        result.push_back(get_extr_max(i));
    }
    return result;
}

std::vector<float> wanda_property::get_extr_tmin_pipe() const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    std::vector<float> result;
    for (int i = 0; i <= _number_of_elements; i++)
    {
        result.push_back(get_extr_tmin(i));
    }
    return result;
}

std::vector<float> wanda_property::get_extr_tmax_pipe() const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    std::vector<float> result;
    for (int i = 0; i <= _number_of_elements; i++)
    {
        result.push_back(get_extr_tmax(i));
    }
    return result;
}

void wanda_property::set_number_of_elements(int number_of_elements)
{
    _number_of_elements = number_of_elements;
    _series_pipe.resize(_number_of_elements + 1, nullptr);
    extr_min.resize(_number_of_elements + 1, nullptr);
    extr_max.resize(_number_of_elements + 1, nullptr);
    extr_tmin.resize(_number_of_elements + 1, nullptr);
    extr_tmax.resize(_number_of_elements + 1, nullptr);
}

// functions below are fast, but they assume that the user knows what type of
// property it is and has called the correct function for that type.
float wanda_property::get_scalar_float() const
{
    if (_spec_status)
    {
        return _scalar;
    }
    throw std::runtime_error(_description + " has no input yet");
}

std::string wanda_property::get_scalar_str()
{
    if (drop_down_list.size() > 0 && _scalar > 0)
        return get_selected_item();
    throw std::runtime_error(_description + " has no input yet");
}

std::vector<float> wanda_property::get_series() const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    if (_series == nullptr)
    {
        throw std::runtime_error("Data is not loaded yet, please load data first");
    }
    return *_series;
}

void wanda_property::set_scalar(float scalar)
{
    if (_wnd_type == wanda_property_types::HIS || _wnd_type == wanda_property_types::NIS ||
        _wnd_type == wanda_property_types::CIS)
    {
        if (scalar < _min_value && scalar != _default_value)
        {
            throw std::runtime_error(_description + " value below minimum value");
        }
    }
    _scalar = scalar;
    _spec_status = true;
    _modified = true;
}

void wanda_property::set_scalar_by_ref(float &scalar)
{
    if (_wnd_type == wanda_property_types::HIS || _wnd_type == wanda_property_types::NIS ||
        _wnd_type == wanda_property_types::CIS)
    {
        if (scalar < _min_value)
        {
            throw std::runtime_error("Value below minimum value");
        }
    }
    _scalar = scalar;
    _spec_status = true;
    _modified = true;
}

void wanda_property::set_scalar(std::string scalarin)
{
    if (_wnd_type == wanda_property_types::NIS || _wnd_type == wanda_property_types::CIS ||
        _wnd_type == wanda_property_types::HIS || _wnd_type == wanda_property_types::GLOV)
    {
        if (_comp_sp_inp_fld == 'C')
        {
            _scalar = std::find(drop_down_list.begin(), drop_down_list.end(), scalarin) - drop_down_list.begin() + 1;
            if (_scalar - 1 >= drop_down_list.size())
            {
                _scalar = 1.0;
            }
            _modified = true;
            _spec_status = true;
        }
    }
}

void wanda_property::set_series_by_ref(std::vector<float> *series)
{
    if (has_series())
    // TODO  access & error checking?
    {
        _series = series;
        return;
    }
    throw std::runtime_error(_description + " property has no series");
}

void wanda_property::set_series_pipe_by_ref(int element, std::vector<float> *series)
{
    // TODO  access & error checking?
    if (!has_series())
    {
        throw std::runtime_error(_description + " property has no series");
    }
    if (_number_of_elements < 1)
    {
        throw std::runtime_error(_description + " is not from a pipe");
    }
    if (element > _number_of_elements)
    {
        throw std::runtime_error(_description + " has only " + std::to_string(_number_of_elements) + " element");
    }
    _series_pipe[element] = series;
}

bool wanda_property::has_scalar() const
{
    if ((_wnd_type == wanda_property_types::NOV || _wnd_type == wanda_property_types::COV ||
         _wnd_type == wanda_property_types::HOV || _wnd_type == wanda_property_types::NIS ||
         _wnd_type == wanda_property_types::CIS || _wnd_type == wanda_property_types::HIS ||
         _wnd_type == wanda_property_types::GLOV || _wnd_type == wanda_property_types::MAO) &&
        !has_table())
    {
        return true;
    }
    return false;
}

bool wanda_property::has_table() const
{
    if (_comp_sp_inp_fld == 'T' || _comp_sp_inp_fld == 'N' || _comp_sp_inp_fld == 'S')
    {
        return true;
    }
    return false;
}

bool wanda_property::has_series() const
{
    if (_wnd_type == wanda_property_types::NOS || _wnd_type == wanda_property_types::COS ||
        _wnd_type == wanda_property_types::HOS || _wnd_type == wanda_property_types::GLOQUANT)
    {
        return true;
    }
    return false;
}

bool wanda_property::is_glo_quant() const
{
    if (_wnd_type == wanda_property_types::GLOQUANT)
    {
        return true;
    }
    return false;
}

void wanda_property::settype()
{
    if (_wdo_post_fix == "HIS")
    {
        _wnd_type = wanda_property_types::HIS;
    }
    else if (_wdo_post_fix == "HCS")
    {
        _wnd_type = wanda_property_types::HCS;
    }
    else if (_wdo_post_fix == "HOS")
    {
        _wnd_type = wanda_property_types::HOS;
    }
    else if (_wdo_post_fix == "HOV")
    {
        _wnd_type = wanda_property_types::HOV;
    }
    else if (_wdo_post_fix == "CIS")
    {
        _wnd_type = wanda_property_types::CIS;
    }
    else if (_wdo_post_fix == "COS")
    {
        _wnd_type = wanda_property_types::COS;
    }
    else if (_wdo_post_fix == "COV")
    {
        _wnd_type = wanda_property_types::COV;
    }
    else if (_wdo_post_fix == "NIS")
    {
        _wnd_type = wanda_property_types::NIS;
    }
    else if (_wdo_post_fix == "NOS")
    {
        _wnd_type = wanda_property_types::NOS;
    }
    else if (_wdo_post_fix == "NOV")
    {
        _wnd_type = wanda_property_types::NOV;
    }
    else if (_wdo_post_fix == "GLOV")
    {
        _wnd_type = wanda_property_types::GLOV;
    }
    else if (_wdo_post_fix == "MAO")
    {
        _wnd_type = wanda_property_types::MAO;
    }
    else
    {
        _wnd_type = wanda_property_types::GLOQUANT;
    }
}

std::string wanda_property::get_list_item(int num_item)
{
    if (num_item < 0)
    {
        return "";
    }
    if (num_item > drop_down_list.size())
    {
        return "";
    }
    return drop_down_list[num_item - 1];
}

void wanda_property::set_list(std::vector<std::string> list)
{
    drop_down_list = list;
}

std::vector<std::string> wanda_property::get_list() const
{
    return drop_down_list;
}

std::string wanda_property::get_selected_item()
{
    if (_scalar == 0)
    {
        return ""; // return empty string since no item is selected
    }
    return drop_down_list[_scalar - 1];
}

bool wanda_property::is_modified() const
{
    return _table.is_modified() || _modified;
    if (has_table())
    {
    }
    return _modified;
}

void wanda_property::set_modified(bool status)
{
    _modified = status;
    if (this->has_table())
        _table.set_modified(status);
}

wanda_table &wanda_property::get_table()
{
    if (has_table())
    {
        return _table;
    }
    throw std::runtime_error(_description + " is not a table");
}

void wanda_property::set_value_from_template(std::unordered_map<std::string, wanda_prop_template>::value_type item)
{

    if (has_table())
    {
        auto &wanda_table = get_table();
        if (item.second.table.size() == 0)
        {
            return;
        }
        auto table = transpose(item.second.table);
        for (auto column : table)
        {
            if (wanda_table.has_description(column[0]))
            {
                if (wanda_table.is_string_column(column[0]))
                {
                    std::vector<std::string> temp_vec(column.begin() + 1, column.end());
                    wanda_table.set_string_column(column[0], temp_vec);
                }
                else
                {
                    std::vector<float> temp_vec;
                    for (int i = 1; i < column.size(); i++)
                    {
                        float value;
                        try
                        {
                            value = std::stof(column[i]);
                        }
                        catch (std::invalid_argument error)
                        {
                            std::cout << error.what();
                        }
                        temp_vec.push_back(value);
                    }
                    wanda_table.set_float_column(column[0], temp_vec);
                }
            }
        }
        return;
    }

    if (item.second.value.empty())
    {
        return;
    }
    if (has_string())
    {
        set_scalar(item.second.value);
    }
    else
    {
        try
        {
            float value = std::stof(item.second.value);
            set_scalar(value);
        }
        catch (std::invalid_argument error)
        {
            std::cout << error.what();
        }
    }
}

std::vector<int> wanda_property::get_view_list_numbers() const
{
    std::vector<int> view_list_numbers;
    int value = _view_list_mask;
    int bit_exp = 0;
    while (value != 0)
    {
        if (value & 1 << bit_exp)
        {
            value -= 1 << bit_exp;
            view_list_numbers.push_back(bit_exp + 1);
        }
        bit_exp++;
    }
    return view_list_numbers;
}

void wanda_property::copy_data(wanda_property prop_org)
{
    if (prop_org.has_scalar())
    {
        set_scalar(prop_org.get_scalar_float());
        return;
    }
    // string
    if (prop_org.has_string())
    {
        set_scalar(prop_org.get_scalar_float());
        set_scalar(prop_org.get_scalar_str());
        return;
    }
    // table
    if (prop_org.has_table())
    {
        auto table_org = prop_org.get_table();
        auto &table_new = get_table();
        table_new.copy_table(table_org);
    }
}

std::vector<float> wanda_property::get_series(int element) const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    if (element <= _number_of_elements)
    {
        if (_series_pipe[element] == nullptr)
        {
            throw std::runtime_error("Data is not loaded yet, please load data first");
        }
        return *(_series_pipe[element]);
    }
    throw std::runtime_error("Element not within total number of elements");
}

std::vector<std::vector<float>> wanda_property::get_series_pipe() const
{
    if (disused)
    {
        throw std::runtime_error("Component is disused");
    }
    if (get_number_of_elements() == 0)
    {
        throw std::runtime_error(get_description() + " does not belong to a pipe");
    }
    std::vector<std::vector<float>> values;
    for (int i = 0; i < _number_of_elements + 1; i++)
    {
        values.push_back(get_series(i));
    }
    return values;
}
