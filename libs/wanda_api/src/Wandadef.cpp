#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <nefis_file.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <wandadef.h>
#include <wandaproperty.h>
#include <deltares_helper_functions.h>

std::unordered_map<std::string, std::unordered_map<std::string, float>> wanda_def::get_unit_list()
{
    std::unordered_map<std::string, std::unordered_map<std::string, float>> unit_list_local;
    int u_table_size = _database.get_int_attribute("UNIT_TABLE", "N_avail_units");
    std::vector<std::string> unit_descr(u_table_size);
    std::vector<std::string> unit_dim(u_table_size);
    std::vector<float> unit_fac(u_table_size);
    _database.get_string_element("UNIT_TABLE", "Unit_descr", {1, u_table_size, 1}, 16, unit_descr);
    _database.get_string_element("UNIT_TABLE", "Unit_dimension", {1, u_table_size, 1}, 12, unit_dim);
    _database.get_float_element("UNIT_TABLE", "Unit_factor", {1, u_table_size, 1}, unit_fac);

    for (int i = 0; i < u_table_size; i++)
    {
        if (unit_list_local.find(unit_dim[i]) == unit_list_local.end())
        {
            // item is not yet in the list
            std::unordered_map<std::string, float> unit_descr_list;
            unit_descr_list[unit_descr[i]] = unit_fac[i];
            unit_list_local[unit_dim[i]] = unit_descr_list;
        }
        else
        {
            // item is in the list
            auto unit_descr_list = unit_list_local[unit_dim[i]];
            unit_descr_list[unit_descr[i]] = unit_fac[i];
            unit_list_local[unit_dim[i]] = unit_descr_list;
        }
    }
    return unit_list_local;
}

std::unordered_map<std::string, std::string> wanda_def::get_case_unit(std::string unit_group)
{
    std::unordered_map<std::string, std::string> case_units;
    int u_table_size = _database.get_int_attribute("UNIT_TABLE", "N_avail_units");
    int u_case_size = _database.get_int_attribute(unit_group, "N_avail_units");
    std::vector<std::string> unit_descr(u_table_size);
    std::vector<std::string> unit_key(u_table_size);
    std::vector<std::string> unit_case_dim(u_case_size);
    std::vector<std::string> unit_case_key(u_case_size);
    _database.get_string_element("UNIT_TABLE", "Unit_descr", {1, u_table_size, 1}, 16, unit_descr);
    _database.get_string_element("UNIT_TABLE", "Unit_key", {1, u_table_size, 1}, 8, unit_key);
    _database.get_string_element(unit_group, "Unit_dimension", {1, u_case_size, 1}, 12, unit_case_dim);
    _database.get_string_element(unit_group, "Unit_key", {1, u_case_size, 1}, 8, unit_case_key);
    for (int i = 0; i < u_case_size; i++)
    {
        int index = get_key_index_array(unit_key, unit_case_key[i]);
        case_units.emplace(unit_case_dim[i], unit_descr[index]);
    }
    return case_units;
}

std::string wanda_def::get_class_sort_key(std::string type)
{
    if (_phys_type_2_classsort.find(type) != _phys_type_2_classsort.end())
    {
        return _phys_type_2_classsort[type];
    }
    if (_node_type_2_classsort.find(type) != _node_type_2_classsort.end())
    {
        return _node_type_2_classsort[type];
    }
    if (_ctrl_type_2_classsort.find(type) != _ctrl_type_2_classsort.end())
    {
        return _ctrl_type_2_classsort[type];
    }
    throw std::invalid_argument(type + " not know in Wandadef");
}

std::vector<std::string> wanda_def::get_core_quants(std::string classsort_key)
{
    std::string groupname = classsort_key + "_GEN";
    std::vector<std::string> core_quant(4);
    _database.get_string_element(groupname, "Glob_core_quant", nefis_file::single_elem_uindex, 8, core_quant);
    return core_quant;
}

std::string wanda_def::get_node_core_quants(std::string classsort_key)
{
    std::string groupname = classsort_key + "_NOD";
    std::vector<std::string> core_quant(1);
    _database.get_string_element(groupname, "Node_core_quant", nefis_file::single_elem_uindex, 8, core_quant);
    return core_quant[0];
}

std::string wanda_def::get_node_type(std::string classsort_key)
{
    std::string groupname = classsort_key + "_NOD";
    std::vector<std::string> node_type(1);
    _database.get_string_element(groupname, "Node_type", nefis_file::single_elem_uindex, 8, node_type);
    return node_type[0];
}

std::vector<int> wanda_def::get_num_input_props(std::string classsort_key)
{
    std::vector<int> number;
    if (is_physical_component(classsort_key))
    {
        std::string groupname = classsort_key + "_HIS";
        number.push_back(_database.get_int_attribute(groupname, "N_his_com"));
        number.push_back(_database.get_int_attribute(groupname, "N_his_ope"));
        groupname = classsort_key + "_HCS";
        number.push_back(_database.get_int_attribute(groupname, "N_hydr_calc_spec"));
    }
    else if (is_control_component(classsort_key))
    {
        std::string groupname = classsort_key + "_CTR";
        number.push_back(0);
        _database.get_int_element(groupname, "N_cis", nefis_file::single_elem_uindex, number);
    }
    else if (is_node(classsort_key))
    {
        std::string groupname = classsort_key + "_NOD";
        number.push_back(0);
        _database.get_int_element(groupname, "N_nis", nefis_file::single_elem_uindex, number);
    }
    return number;
}

void wanda_def::get_ip_fld_his(std::string classsort_key, std::vector<std::string> &com, std::vector<std::string> &ope)
{
    if (!is_physical_component(classsort_key))
    {
        throw std::invalid_argument(classsort_key + " is not a physical component");
    }
    std::string groupname = classsort_key + "_HIS";
    int N_inputs = _database.get_int_attribute(groupname, "N_hydr_inp_specs");
    std::vector<std::string> comp_sp_inp_fld(N_inputs);
    _database.get_string_element(groupname, "Comp_sp_inp_fld", {1, N_inputs, 1}, 1, comp_sp_inp_fld);
    std::vector<std::string> comp_spec_code(N_inputs);
    _database.get_string_element(groupname, "Comp_spec_code", {1, N_inputs, 1}, 1, comp_spec_code);
    for (int i = 0; i < N_inputs; i++)
    {
        if (comp_spec_code[i][0] == 'C')
        {
            com.push_back(comp_sp_inp_fld[i]);
        }
        else
        {
            ope.push_back(comp_sp_inp_fld[i]);
        }
    }
    while (com.size() < 36)
    {
        com.push_back("");
    }
    while (ope.size() < 36)
    {
        ope.push_back("");
    }
}

void wanda_def::get_ip_fld_his(std::string classsort_key, std::vector<std::string> &com)
{
    if (!is_control_component(classsort_key))
    {
        throw std::invalid_argument(classsort_key + " is not a physical component");
    }
    std::string groupname = classsort_key + "_CTR";
    int N_inputs = _database.get_int_attribute(groupname, "N_ctr_specs");
    com.resize(std::max(N_inputs, 1));
    _database.get_string_element(groupname, "Comp_sp_inp_fld", {1, std::max(N_inputs, 1), 1}, 1, com);
}

std::vector<std::string> wanda_def::get_ctrl_in_type(std::string classsort_key)
{
    std::string groupname = classsort_key + "_CTR";
    std::vector<std::string> in_type(16);
    _database.get_string_element(groupname, "Chanl_in_type", nefis_file::single_elem_uindex, 8, in_type);
    return in_type;
}

std::vector<std::string> wanda_def::get_ctrl_out_type(std::string classsort_key)
{
    std::string groupname = classsort_key + "_CTR";
    std::vector<std::string> out_type(16);
    _database.get_string_element(groupname, "Chanl_out_type", nefis_file::single_elem_uindex, 8, out_type);
    return out_type;
}

std::vector<int> wanda_def::get_num_input_chanl(std::string classsort_key)
{
    std::vector<int> n_chan_in(1);
    std::string groupname = classsort_key + "_CTR";
    _database.get_int_element(groupname, "N_input_chanl", nefis_file::single_elem_uindex, n_chan_in);
    return n_chan_in;
}

std::vector<int> wanda_def::get_num_output_chanl(std::string classsort_key)
{
    std::vector<int> n_chan_out(1);
    std::string groupname = classsort_key + "_CTR";
    _database.get_int_element(groupname, "N_output_chanl", nefis_file::single_elem_uindex, n_chan_out);
    return n_chan_out;
}

bool wanda_def::check_def_mask(std::string class_sort_key, char mask)
{
    std::string def_mask = get_default_mask(class_sort_key);
    auto res = std::find(def_mask.begin(), def_mask.end(), mask);
    return res != def_mask.end();
}

bool wanda_def::is_obsolete(std::string class_sort_key)
{
    return check_def_mask(class_sort_key, 'X');
}

bool wanda_def::is_prototype(std::string class_sort_key)
{
    return check_def_mask(class_sort_key, 'P');
}

bool wanda_def::is_special(std::string class_sort_key)
{
    return check_def_mask(class_sort_key, 'S');
}

std::vector<std::string> wanda_def::get_possible_phys_comp_type()
{
    std::string group = "H_COMP_AVAILABLE";
    int size = _database.get_int_attribute(group, "N_avail_H_class");
    std::vector<std::vector<std::string>> avail_classes(size, std::vector<std::string>(250));
    _database.get_string_element(group, "Class_sorts", {1, size, 1}, {1, 250, 1}, 8, avail_classes);
    std::vector<int> n_sorts(size);
    _database.get_int_element(group, "N_sorts", {1, size, 1}, n_sorts);
    std::vector<std::string> avail_type;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < n_sorts[i]; j++)
        {
            // only add the component when it is not obsolete
            if (!is_obsolete(avail_classes[i][j]) && !is_prototype(avail_classes[i][j]) &&
                !is_special(avail_classes[i][j]))
            {
                avail_type.push_back(get_type_name_phys(avail_classes[i][j]));
            }
        }
    }
    return avail_type;
}

std::vector<std::string> wanda_def::get_possible_ctrl_comp_type()
{
    std::string group = "C_COMP_AVAILABLE";
    int size = _database.get_int_attribute(group, "N_avail_C_class");
    std::vector<std::string> avail_classes(size);
    _database.get_string_element(group, "Class_name", {1, size, 1}, 8, avail_classes);
    std::vector<std::string> avail_type;
    for (int i = 0; i < size; i++)
    {
        // only add the component when it is not obsolete
        if (!is_obsolete(avail_classes[i]) && !is_prototype(avail_classes[i]) && !is_special(avail_classes[i]))
        {
            avail_type.push_back(get_type_name_ctrl(avail_classes[i]));
        }
    }
    return avail_type;
}

std::vector<std::string> wanda_def::get_possible_node_type()
{
    std::string group = "H_NODE_AVAILABLE";
    int size = _database.get_int_attribute(group, "N_avail_N_class");
    std::vector<std::string> avail_classes(size);
    _database.get_string_element(group, "Class_name", {1, size, 1}, 8, avail_classes);
    std::vector<std::string> avail_type;
    for (int i = 0; i < size; i++)
    {
        if (!is_obsolete(avail_classes[i]) && !is_prototype(avail_classes[i]) && !is_special(avail_classes[i]))
        {
            avail_type.push_back(get_type_name_node(avail_classes[i]));
        }
    }
    return avail_type;
}

int wanda_def::get_element_size(std::string &element)
{
    return _database.get_element_size(element);
}

wanda_def::~wanda_def()
{
    _database.close();
}

wanda_def::wanda_def(std::string data_path) : _data_path(data_path)
{
    if (_data_path[_data_path.length() - 1] != '\\') // ensure _data_path ends with a backslash
    {
        _data_path.append("\\");
    }
    std::string wanda_dat = _data_path + "WandaDef.dat";
#ifdef DEBUG
    std::cout << "Wandadef loaded from: " << wanda_dat << '\n';
#endif
    _database.set_file(wanda_dat);
    _database.open('r'); // open WandaDef as readonly file to prevent
                         // 'access-denied' issues
    load_version_number();

    load_class_sort_keys();
    load_table_types();
    load_numcol_types();
    load_string_col_types();
    load_unit_list();
    std::vector<std::string> related_his(1);
    std::vector<std::string> action_table_type_key(1);
    _num_phys_comp = 0;
    _num_ctrl_comp = 0;
}

void wanda_def::initialize(std::string data_path)
{
    if (!initialized) // if this instance is already initialized, skip
                      // initialisation
    {

        initialized = true;
    }
}

void wanda_def::load_class_sort_keys()
{
    int nclasses = _database.get_int_attribute("H_COMP_AVAILABLE", "N_avail_H_class");
    std::vector<std::string> classnames(nclasses);
    _database.get_string_element("H_COMP_AVAILABLE", "Class_name", {1, nclasses, 1}, 8, classnames);

    std::vector<int> n_sorts(nclasses);
    _database.get_int_element("H_COMP_AVAILABLE", "N_sorts", {1, nclasses, 1}, n_sorts);
    int nClassSorts = 0;
    for (int i = 0; i < nclasses; i++)
    {
        _phys_class_names.insert(classnames[i]);
        nClassSorts += n_sorts[i];
    }

    for (int i = 1; i <= nclasses; i++)
    {
        std::vector<std::vector<std::string>> resultarray(1, std::vector<std::string>(250));
        _database.get_string_element("H_COMP_AVAILABLE", "Class_sorts", {i, i, 1}, {1, 250, 1}, 8, resultarray);
        for (int j = 0; j < n_sorts[i - 1]; j++)
        {
            _phys_class_sort_keys.insert(resultarray[0][j]);
            std::vector<std::string> type(1);
            _database.get_string_element(resultarray[0][j] + "_GEN", "Type_name", nefis_file::single_elem_uindex, 48,
                                         type);
            _phys_type_2_classsort[type[0]] = resultarray[0][j];
        }
    }
    _num_phys_comp = int(_phys_class_sort_keys.size());

    ////Control components
    nclasses = _database.get_int_attribute("C_COMP_AVAILABLE", "N_avail_C_class");
    std::vector<std::string> ctrlclass(nclasses);
    _database.get_string_element("C_COMP_AVAILABLE", "Class_name", {1, nclasses, 1}, 8, ctrlclass);
    for (int i = 0; i < nclasses; i++)
    {
        _control_class_sort_keys.insert(ctrlclass[i]);
        std::vector<std::string> type(1);
        _database.get_string_element(ctrlclass[i] + "_CTR", "Type_name", nefis_file::single_elem_uindex, 48, type);
        _ctrl_type_2_classsort[type[0]] = ctrlclass[i];
    }

    _num_ctrl_comp = int(_control_class_sort_keys.size());

    // nodes
    nclasses = _database.get_int_attribute("H_NODE_AVAILABLE", "N_avail_N_class");
    std::vector<std::string> node_class(nclasses);
    _database.get_string_element("H_NODE_AVAILABLE", "Class_name", {1, nclasses, 1}, 8, node_class);
    for (int i = 0; i < nclasses; i++)
    {
        _node_class_sort_keys.insert(node_class[i]);
        std::vector<std::string> type(1);
        _database.get_string_element(node_class[i] + "_NOD", "Type_name", nefis_file::single_elem_uindex, 48, type);
        _node_type_2_classsort[type[0]] = node_class[i];
    }
}

void wanda_def::load_table_types()
{
    int size_group = _database.get_int_attribute("TABLE_TYPES", "N_table_types");
    std::vector<std::string> table_type_key(size_group);
    std::vector<std::vector<std::string>> table_descr(size_group, std::vector<std::string>(2));
    std::vector<int> max_points_req(size_group);
    std::vector<int> min_points_req(size_group);
    std::vector<std::vector<float>> max_value(size_group, std::vector<float>(2));
    std::vector<std::vector<float>> min_value(size_group, std::vector<float>(2));
    std::vector<std::vector<std::string>> table_unit_dim(size_group, std::vector<std::string>(2));
    nefis_uindex all_elems_uindex = {1, size_group, 1};
    _database.get_string_element("TABLE_TYPES", "Table_type_key", all_elems_uindex, 8, table_type_key);
    _database.get_string_element("TABLE_TYPES", "Table_descr", all_elems_uindex, {1, 2, 1}, 0, table_descr);
    _database.get_int_element("TABLE_TYPES", "Max_points_req", all_elems_uindex, max_points_req);
    _database.get_int_element("TABLE_TYPES", "Min_points_req", all_elems_uindex, min_points_req);
    _database.get_float_element("TABLE_TYPES", "Max_value2", all_elems_uindex, {1, 2, 1}, max_value, true);
    _database.get_float_element("TABLE_TYPES", "Min_value2", all_elems_uindex, {1, 2, 1}, min_value, true);
    _database.get_string_element("TABLE_TYPES", "Table_unit_dim", all_elems_uindex, {1, 2, 1}, 12, table_unit_dim);
    for (int i = 0; i < size_group; i++)
    {
        table_info[table_type_key[i]] = wanda_tab_info();
        for (int j = 0; j < 2; j++)
        {
            table_info[table_type_key[i]].description.push_back(table_descr[i][j]);
            table_info[table_type_key[i]].min_val.push_back(min_value[i][j]);
            table_info[table_type_key[i]].max_val.push_back(max_value[i][j]);
            table_info[table_type_key[i]].min_points_req.push_back(min_points_req[i]);
            table_info[table_type_key[i]].max_points_req.push_back(max_points_req[i]);
            table_info[table_type_key[i]].unit_dim.push_back(table_unit_dim[i][j]);
        }
    }
}

void wanda_def::load_numcol_types()
{
    int size_group = _database.get_int_attribute("NUMCOLUMN_TYPES", "N_column_types");
    std::vector<std::string> table_type_key(size_group);
    std::vector<std::string> table_descr(size_group);
    std::vector<int> max_points_req(size_group);
    std::vector<int> min_points_req(size_group);
    std::vector<float> max_value(size_group);
    std::vector<float> min_value(size_group);
    std::vector<std::string> table_unit_dim(size_group);
    _database.get_string_element("NUMCOLUMN_TYPES", "Column_type_key", {1, size_group, 1}, 8, table_type_key);
    _database.get_string_element("NUMCOLUMN_TYPES", "Column_descr", {1, size_group, 1}, 0, table_descr);
    _database.get_int_element("NUMCOLUMN_TYPES", "Max_points_req", {1, size_group, 1}, max_points_req);
    _database.get_int_element("NUMCOLUMN_TYPES", "Min_points_req", {1, size_group, 1}, min_points_req);
    _database.get_float_element("NUMCOLUMN_TYPES", "Max_value", {1, size_group, 1}, max_value);
    _database.get_float_element("NUMCOLUMN_TYPES", "Min_value", {1, size_group, 1}, min_value);
    _database.get_string_element("NUMCOLUMN_TYPES", "Column_unit_dim", {1, size_group, 1}, 12, table_unit_dim);
    for (int i = 0; i < size_group; i++)
    {
        num_col_info[table_type_key[i]] = wanda_tab_info();
        num_col_info[table_type_key[i]].description.push_back(table_descr[i]);
        num_col_info[table_type_key[i]].min_val.push_back(min_value[i]);
        num_col_info[table_type_key[i]].max_val.push_back(max_value[i]);
        num_col_info[table_type_key[i]].min_points_req.push_back(min_points_req[i]);
        num_col_info[table_type_key[i]].max_points_req.push_back(max_points_req[i]);
        num_col_info[table_type_key[i]].unit_dim.push_back(table_unit_dim[i]);
    }
}

void wanda_def::load_string_col_types()
{
    int size_group = _database.get_int_attribute("CHARCOLUMN_TYPES", "N_column_types");
    std::vector<std::string> table_type_key(size_group);
    std::vector<std::string> table_descr(size_group);
    std::vector<int> max_points_req(size_group);
    std::vector<int> min_points_req(size_group);
    _database.get_string_element("CHARCOLUMN_TYPES", "Column_type_key", {1, size_group, 1}, 8, table_type_key);
    _database.get_string_element("CHARCOLUMN_TYPES", "Column_descr", {1, size_group, 1}, 0, table_descr);
    _database.get_int_element("CHARCOLUMN_TYPES", "Max_points_req", {1, size_group, 1}, max_points_req);
    _database.get_int_element("CHARCOLUMN_TYPES", "Min_points_req", {1, size_group, 1}, min_points_req);
    for (int i = 0; i < size_group; i++)
    {
        string_col_info[table_type_key[i]] = wanda_tab_info();
        string_col_info[table_type_key[i]].description.push_back(table_descr[i]);
        string_col_info[table_type_key[i]].min_val.push_back(0.0);
        string_col_info[table_type_key[i]].max_val.push_back(0.0);
        string_col_info[table_type_key[i]].min_points_req.push_back(min_points_req[i]);
        string_col_info[table_type_key[i]].max_points_req.push_back(max_points_req[i]);
        string_col_info[table_type_key[i]].unit_dim.push_back("");
    }
}

std::unordered_map<std::string, wanda_property> wanda_def::get_physical_input_properties(std::string classsort_key)
{
    if (phys_input_props.find(classsort_key) == phys_input_props.end())
    {
        std::string groupname = classsort_key + "_GEN";
        std::vector<std::string> related_his(1);
        std::vector<std::string> action_table_type_key(1);
        _database.get_string_element(groupname, "Related_his", nefis_file::single_elem_uindex, 30, related_his);
        _database.get_string_element(groupname, "Table_type_key", nefis_file::single_elem_uindex, 8,
                                     action_table_type_key);

        groupname = classsort_key + "_HIS";
        int his_com = _database.get_int_attribute(groupname, "N_his_com");
        int his_oper = _database.get_int_attribute(groupname, "N_his_ope");
        int hydr_inp_specs = his_com + his_oper;

        if (hydr_inp_specs > 0)
        {
            std::vector<std::string> spec_descr(hydr_inp_specs);
            std::vector<std::string> spec_descr_short(hydr_inp_specs);
            std::vector<std::string> comp_spec_code(hydr_inp_specs);
            std::vector<std::string> comp_sp_inp_fld(hydr_inp_specs);
            std::vector<std::string> input_type_code(hydr_inp_specs);
            std::vector<std::string> table_type_key(hydr_inp_specs);
            std::vector<std::string> unit_dim(hydr_inp_specs);
            std::vector<std::vector<std::string>> lists(hydr_inp_specs, std::vector<std::string>(10));
            std::vector<int> N_toggle_fields(hydr_inp_specs);
            std::vector<float> min_value(hydr_inp_specs);
            std::vector<float> max_value(hydr_inp_specs);
            std::vector<float> default_value(hydr_inp_specs);
            std::vector<std::string> list_dependency(hydr_inp_specs);
            std::vector<int> view_list_mask(hydr_inp_specs);
            std::vector<int> view_mask(hydr_inp_specs);
            _database.get_string_element(groupname, "Spec_descr", {1, hydr_inp_specs, 1}, 30, spec_descr);
            _database.get_string_element(groupname, "Spec_descr_short", {1, hydr_inp_specs, 1}, 3, spec_descr_short);
            _database.get_string_element(groupname, "Comp_spec_code", {1, hydr_inp_specs, 1}, 1, comp_spec_code);
            _database.get_string_element(groupname, "Comp_sp_inp_fld", {1, hydr_inp_specs, 1}, 1, comp_sp_inp_fld);
            _database.get_string_element(groupname, "Input_type_code", {1, hydr_inp_specs, 1}, 1, input_type_code);
            _database.get_string_element(groupname, "Table_type_key", {1, hydr_inp_specs, 1}, 8, table_type_key);
            _database.get_string_element(groupname, "Unit_dimension", {1, hydr_inp_specs, 1}, 12, unit_dim);
            _database.get_string_element(groupname, "Toggle_fields", {1, hydr_inp_specs, 1}, {1, 10, 1}, 16, lists);
            _database.get_int_element(groupname, "N_toggle_fields", {1, hydr_inp_specs, 1}, N_toggle_fields);
            _database.get_float_element(groupname, "Min_value", {1, hydr_inp_specs, 1}, min_value);
            _database.get_float_element(groupname, "Max_value", {1, hydr_inp_specs, 1}, max_value);
            _database.get_float_element(groupname, "Default_value", {1, hydr_inp_specs, 1}, default_value);
            _database.get_string_element(groupname, "List_dependency", {1, hydr_inp_specs, 1}, 30, list_dependency);
            _database.get_int_element(groupname, "View_list_mask", {1, hydr_inp_specs, 1}, view_list_mask);
            _database.get_int_element(groupname, "View_mask", {1, hydr_inp_specs, 1}, view_mask);
            for (int i = 0; i < hydr_inp_specs; i++)
            {
                int index;
                if (i < his_com)
                    index = i; // Common specs
                else
                    index = i - his_com; // Operational specs
                if (phys_input_props[classsort_key].find(spec_descr[i]) == phys_input_props[classsort_key].end())
                {
                    phys_input_props[classsort_key][spec_descr[i]] = wanda_property(
                        index, spec_descr[i], comp_spec_code[i][0], comp_sp_inp_fld[i][0], "HIS", unit_dim[i],
                        wanda_property_types::HIS, spec_descr_short[i], default_value[i], min_value[i], max_value[i],
                        list_dependency[i], view_list_mask[i], input_type_code[i][0], view_mask[i]);
                }
                auto &property = phys_input_props[classsort_key][spec_descr[i]];
                if (property.get_property_spec_inp_fld() == 'C')
                {
                    std::vector<std::string> list(lists[i].begin(), lists[i].begin() + N_toggle_fields[i]);
                    property.set_list(list);
                }
                else if (property.get_property_spec_inp_fld() != 'R' && property.get_property_spec_inp_fld() != 'I' &&
                         property.get_property_spec_inp_fld() != 'C')
                {
                    // setting additional settings for table, numcolumns & stringcolumns
                    wanda_tab_info tab_info;
                    int counter = 0;
                    if (comp_sp_inp_fld[i][0] == 'S')
                    {
                        tab_info = string_col_info[table_type_key[i]];
                        counter = 1;
                    }
                    else if (comp_sp_inp_fld[i][0] == 'N')
                    {
                        tab_info = num_col_info[table_type_key[i]];
                        counter = 1;
                    }
                    else if (comp_sp_inp_fld[i][0] == 'T')
                    {
                        tab_info = table_info[table_type_key[i]];
                        counter = 2;
                    }
                    wanda_table &table = property.get_table();
                    for (int j = 0; j < counter; j++)
                    {
                        table.add_column(tab_info.description[j], tab_info.unit_dim[j], "Unrefrnc",
                                         comp_sp_inp_fld[i][0], index, j,
                                         tab_info.description[j == 0 ? counter - 1 : 0], comp_spec_code[i][0]);
                    }
                }
            }
            // adding action table
            if (!(related_his[0].compare("None") == 0) && !(related_his[0].compare("") == 0))
            {
                auto tab_info = table_info[action_table_type_key[0]];
                wanda_property prop(-999, "Action table", 'N', 'T', "HIS", tab_info.unit_dim[0],
                                    wanda_property_types::HIS, "AT");
                wanda_table &table = prop.get_table();
                for (int j = 0; j < 2; j++)
                {
                    table.add_column(tab_info.description[j], tab_info.unit_dim[j], "Unrefrnc", 'T', -999, j,
                                     tab_info.description[j == 0 ? 1 : 0], 'C');
                }
                prop.set_property_spec_inp_fld('T');
                phys_input_props[classsort_key]["Action table"] = prop;
            }
        }
    }

    return phys_input_props[classsort_key];
}

std::unordered_map<std::string, wanda_property> wanda_def::get_physical_calc_properties(std::string classsort_key)
{
    std::string groupname = classsort_key + "_HCS";

    int N_hydr_calc_spec = _database.get_int_attribute(groupname, "N_hydr_calc_spec");
    std::unordered_map<std::string, wanda_property> propertylist;

    if (N_hydr_calc_spec > 0)
    {
        std::vector<std::string> spec_descr(N_hydr_calc_spec);
        _database.get_string_element(groupname, "Spec_descr", {1, N_hydr_calc_spec, 1}, 30, spec_descr);
        std::vector<std::string> spec_descr_short(N_hydr_calc_spec);
        _database.get_string_element(groupname, "Spec_descr_short", {1, N_hydr_calc_spec, 1}, 3, spec_descr_short);
        std::vector<std::string> Comp_sp_inp_fld(N_hydr_calc_spec);
        _database.get_string_element(groupname, "Comp_sp_inp_fld", {1, N_hydr_calc_spec, 1}, 1, Comp_sp_inp_fld);
        std::vector<std::string> unit_dim(N_hydr_calc_spec);
        _database.get_string_element(groupname, "Unit_dimension", {1, N_hydr_calc_spec, 1}, 12, unit_dim);

        for (int i = 0; i < N_hydr_calc_spec; i++)
        {
            propertylist[spec_descr[i]] = wanda_property(i, spec_descr[i], '0', Comp_sp_inp_fld[i][0], "HCS",
                                                         unit_dim[i], wanda_property_types::HCS, spec_descr_short[i]);
        }
    }
    return propertylist;
}

std::unordered_map<std::string, wanda_property> wanda_def::get_physical_output_properties(std::string classsort_key)
{
    if (phys_output_props.find(classsort_key) == phys_output_props.end())
    {
        int numproperties = -1;
        std::string groupname = classsort_key + "_GEN";
        std::vector<int> h_node_counts(1);
        std::vector<std::string> glob_core_quants(4);
        std::vector<std::string> glob_rest_quants(4);
        std::vector<std::string> glob_outp_quants(4);
        std::vector<std::string> comp_type(1);
        int n_avail_quants = _database.get_int_attribute("GLOBAL_QUANTITIE", "N_avail_quants");
        std::vector<std::string> quantsymbols(n_avail_quants);
        std::vector<std::string> quantnames(n_avail_quants);
        std::vector<std::string> WDO_postfixes(n_avail_quants);
        std::vector<std::string> unit_dims(n_avail_quants);
        _database.get_string_element(groupname, "Glob_core_quant", nefis_file::single_elem_uindex, 8, glob_core_quants);
        _database.get_string_element(groupname, "Glob_rest_quant", nefis_file::single_elem_uindex, 8, glob_rest_quants);
        for (int i = 0; i < 4; i++)
        {
            glob_outp_quants[i] = glob_core_quants[i] + glob_rest_quants[i];
        }

        _database.get_string_element(groupname, "Comp_type", nefis_file::single_elem_uindex, 8, comp_type);
        _database.get_int_element(groupname, "H_node_count", nefis_file::single_elem_uindex, h_node_counts);
        _database.get_string_element("GLOBAL_QUANTITIE", "Quantity_symbol", {1, n_avail_quants, 1}, 1, quantsymbols);
        _database.get_string_element("GLOBAL_QUANTITIE", "Quantity_name", {1, n_avail_quants, 1}, 30, quantnames);
        _database.get_string_element("GLOBAL_QUANTITIE", "WDO_postfix", {1, n_avail_quants, 1}, 11, WDO_postfixes);
        _database.get_string_element("GLOBAL_QUANTITIE", "Unit_dimension", {1, n_avail_quants, 1}, 12, unit_dims);

        int H_node_count = h_node_counts[0];
        for (int i = 0; i < H_node_count; ++i)
        {
            std::string glob_outp_quant = glob_outp_quants[i];
            for (size_t j = 0; j < to_up(glob_outp_quant).size(); ++j)
            {

                __int64 index = std::find(quantsymbols.begin(), quantsymbols.end(), glob_outp_quant.substr(j, 1)) -
                                quantsymbols.begin();
                if (index == quantsymbols.size())
                    throw std::out_of_range("Cannot find " + glob_outp_quant.substr(j, 1) + " in global quantity list");
                if (quantnames[index] == "Composition")
                {
                    for (int k = 1; k <= max_num_species; k++)
                    {
                        std::string spec_descr =
                            std::string(quantnames[index] + " " + std::to_string(k) + " " + std::to_string(i + 1));
                        numproperties++;
                        phys_output_props[classsort_key][spec_descr] =
                            wanda_property(numproperties, spec_descr, '0', 'G', WDO_postfixes[index], unit_dims[index],
                                           wanda_property_types::GLOQUANT, glob_outp_quant.substr(j, 1));
                        phys_output_props[classsort_key][spec_descr].set_species_number(k);
                        phys_output_props[classsort_key][spec_descr].set_connection_point(i);
                    }
                }
                else
                {
                    std::string spec_descr = std::string(quantnames[index] + " " + std::to_string(i + 1));
                    numproperties++;
                    phys_output_props[classsort_key][spec_descr] =
                        wanda_property(numproperties, spec_descr, '0', 'G', WDO_postfixes[index], unit_dims[index],
                                       wanda_property_types::GLOQUANT, glob_outp_quant.substr(j, 1));
                    phys_output_props[classsort_key][spec_descr].set_connection_point(i);
                }
                std::string spec_descr = std::string(quantnames[index]);
                if (comp_type[0] == "PIPE" &&
                    phys_output_props[classsort_key].find(spec_descr) == phys_output_props[classsort_key].end())
                {
                    // component is a pipe add also internal output properties
                    if (quantnames[index] == "Composition")
                    {
                        for (int k = 1; k <= max_num_species; k++)
                        {
                            spec_descr = std::string(quantnames[index] + " " + std::to_string(k));
                            numproperties++;
                            phys_output_props[classsort_key][spec_descr] = wanda_property(
                                numproperties, spec_descr, '0', 'G', WDO_postfixes[index], unit_dims[index],
                                wanda_property_types::GLOQUANT, glob_outp_quant.substr(j, 1));
                            phys_output_props[classsort_key][spec_descr].set_con_point_quant(false);
                            phys_output_props[classsort_key][spec_descr].set_species_number(k);
                        }
                    }
                    else
                    {
                        spec_descr = std::string(quantnames[index]);
                        numproperties++;
                        phys_output_props[classsort_key][spec_descr] =
                            wanda_property(numproperties, spec_descr, '0', 'G', WDO_postfixes[index], unit_dims[index],
                                           wanda_property_types::GLOQUANT, glob_outp_quant.substr(j, 1));
                        phys_output_props[classsort_key][spec_descr].set_con_point_quant(false);
                    }
                }
            }
        }

        // HOS properties
        groupname = classsort_key + "_HOS";
        int n_hydr_out_specs = _database.get_int_attribute(groupname, "N_hydr_out_specs");
        if (n_hydr_out_specs > 0)
        {
            std::vector<std::string> spec_descr(n_hydr_out_specs);
            std::vector<std::string> spec_descr_short(n_hydr_out_specs);
            std::vector<std::string> comp_sp_inp_fld(n_hydr_out_specs);
            std::vector<std::string> unit_dim_hos(n_hydr_out_specs);
            _database.get_string_element(groupname, "Spec_descr", {1, n_hydr_out_specs, 1}, 30, spec_descr);
            _database.get_string_element(groupname, "Spec_descr_short", {1, n_hydr_out_specs, 1}, 3, spec_descr_short);
            _database.get_string_element(groupname, "Comp_sp_inp_fld", {1, n_hydr_out_specs, 1}, 1, comp_sp_inp_fld);
            _database.get_string_element(groupname, "Unit_dimension", {1, n_hydr_out_specs, 1}, 12, unit_dim_hos);

            for (int i = 0; i < n_hydr_out_specs; i++)
            {
                numproperties++;
                phys_output_props[classsort_key][spec_descr[i]] =
                    wanda_property(numproperties, spec_descr[i], '0', comp_sp_inp_fld[i][0], "HOS", unit_dim_hos[i],
                                   wanda_property_types::HOS, spec_descr_short[i]);
                ;
                phys_output_props[classsort_key][spec_descr[i]].set_hos_index(i);
            }
        }
        // HOV properties
        groupname = classsort_key + "_HOV";
        n_hydr_out_specs = _database.get_int_attribute(groupname, "N_hydr_out_specs");
        if (n_hydr_out_specs > 0)
        {
            std::vector<std::string> spec_descr(n_hydr_out_specs);
            std::vector<std::string> spec_descr_short(n_hydr_out_specs);
            std::vector<std::string> comp_sp_inp_fld(n_hydr_out_specs);
            std::vector<std::string> unit_dim_hov(n_hydr_out_specs);
            _database.get_string_element(groupname, "Spec_descr", {1, n_hydr_out_specs, 1}, 30, spec_descr);
            _database.get_string_element(groupname, "Spec_descr_short", {1, n_hydr_out_specs, 1}, 3, spec_descr_short);
            _database.get_string_element(groupname, "Comp_sp_inp_fld", {1, n_hydr_out_specs, 1}, 1, comp_sp_inp_fld);
            _database.get_string_element(groupname, "Unit_dimension", {1, n_hydr_out_specs, 1}, 12, unit_dim_hov);
            for (int i = 0; i <= n_hydr_out_specs - 1; i++)
            {
                numproperties++;
                phys_output_props[classsort_key][spec_descr[i]] =
                    wanda_property(numproperties, spec_descr[i], '0', comp_sp_inp_fld[i][0], "HOV", unit_dim_hov[i],
                                   wanda_property_types::HOV, spec_descr_short[i]);
                phys_output_props[classsort_key][spec_descr[i]].set_hos_index(i);
            }
        }
    }
    return phys_output_props[classsort_key];
}

std::unordered_map<std::string, wanda_property> wanda_def::get_control_input_properties(std::string classsort_key)
{
    std::string groupname = classsort_key + "_CTR";
    std::unordered_map<std::string, wanda_property> propertylist;
    int n_cis = _database.get_int_attribute(groupname, "N_ctr_specs");
    if (n_cis > 0)
    {
        std::vector<std::string> spec_descr(n_cis);
        std::vector<std::string> spec_descr_short(n_cis);
        std::vector<std::string> comp_sp_inp_fld(n_cis);
        std::vector<std::string> unit_dim(comp_sp_inp_fld);
        std::vector<std::string> table_type_key(n_cis);
        std::vector<std::vector<std::string>> lists(n_cis, std::vector<std::string>(10));
        std::vector<int> N_toggle_fields(n_cis);
        std::vector<float> min_value(n_cis);
        std::vector<float> max_value(n_cis);
        std::vector<float> default_value(n_cis);
        std::vector<std::string> list_dependency(n_cis);
        std::vector<std::string> input_type_code(n_cis);
        std::vector<int> view_list_mask(n_cis);
        std::vector<int> view_mask(n_cis);
        nefis_uindex n_cis_index = {1, n_cis, 1};
        _database.get_string_element(groupname, "Spec_descr", n_cis_index, 30, spec_descr);
        _database.get_string_element(groupname, "Spec_descr_short", n_cis_index, 3, spec_descr_short);
        _database.get_string_element(groupname, "Comp_sp_inp_fld", n_cis_index, 1, comp_sp_inp_fld);
        _database.get_string_element(groupname, "Unit_dimension", n_cis_index, 12, unit_dim);
        _database.get_string_element(groupname, "Table_type_key", n_cis_index, 8, table_type_key);

        _database.get_string_element(groupname, "Toggle_fields", {1, n_cis, 1}, {1, 10, 1}, 16, lists);
        _database.get_int_element(groupname, "N_toggle_fields", {1, n_cis, 1}, N_toggle_fields);
        _database.get_float_element(groupname, "Min_value", {1, n_cis, 1}, min_value);
        _database.get_float_element(groupname, "Max_value", {1, n_cis, 1}, max_value);
        _database.get_float_element(groupname, "Default_value", {1, n_cis, 1}, default_value);
        _database.get_string_element(groupname, "List_dependency", n_cis_index, 30, list_dependency);
        _database.get_int_element(groupname, "View_list_mask", {1, n_cis, 1}, view_list_mask);
        _database.get_int_element(groupname, "View_mask", {1, n_cis, 1}, view_mask);
        _database.get_string_element(groupname, "Input_type_code", n_cis_index, 1, input_type_code);
        for (int i = 0; i < n_cis; i++)
        {
            propertylist[spec_descr[i]] = wanda_property(
                i, spec_descr[i], 'C', comp_sp_inp_fld[i][0], "CIS", unit_dim[i], wanda_property_types::CIS,
                spec_descr_short[i], default_value[i], min_value[i], max_value[i], list_dependency[i],
                view_list_mask[i], input_type_code[i][0], view_mask[i]);
            propertylist[spec_descr[i]].set_hos_index(i);
            if (propertylist[spec_descr[i]].get_property_spec_inp_fld() == 'C')
            {
                std::vector<std::string> list(lists[i].begin(), lists[i].begin() + N_toggle_fields[i]);
                propertylist[spec_descr[i]].set_list(list);
            }
            if (propertylist[spec_descr[i]].get_property_spec_inp_fld() != 'R' &&
                propertylist[spec_descr[i]].get_property_spec_inp_fld() != 'I' &&
                propertylist[spec_descr[i]].get_property_spec_inp_fld() != 'C')
            {
                // setting additional settings for table
                wanda_tab_info tab_info;
                int counter = 0;
                if (comp_sp_inp_fld[i][0] == 'S')
                {
                    tab_info = string_col_info[table_type_key[i]];
                    counter = 1;
                }
                else if (comp_sp_inp_fld[i][0] == 'N')
                {
                    tab_info = num_col_info[table_type_key[i]];
                    counter = 1;
                }
                else if (comp_sp_inp_fld[i][0] == 'T')
                {
                    tab_info = table_info[table_type_key[i]];
                    counter = 2;
                }
                wanda_table &table = propertylist[spec_descr[i]].get_table();
                for (int j = 0; j < counter; j++)
                {
                    table.add_column(tab_info.description[j], tab_info.unit_dim[j], "Unrefrnc", comp_sp_inp_fld[i][0],
                                     i, j, tab_info.description[j == 0 ? counter - 1 : 0], 'C');
                }
            }
        }
    }
    return propertylist;
}

std::unordered_map<std::string, wanda_property> wanda_def::get_control_output_properties(std::string classsort_key)
{
    int numproperties = -1;
    std::unordered_map<std::string, wanda_property> propertylist;
    std::string groupname = classsort_key + "_CTR";
    std::vector<int> n_output_chanl(1);
    _database.get_int_element(groupname, "N_output_chanl", nefis_file::single_elem_uindex, n_output_chanl);
    for (int i = 0; i < n_output_chanl[0]; i++)
    {
        numproperties++;
        std::string spec_descr;
        if (n_output_chanl[0] == 1)
            spec_descr = "Output channel";
        else
            spec_descr = "Output channel " + std::to_string(i + 1);
        propertylist[spec_descr] = wanda_property(numproperties, spec_descr, '0', 'R', "CHANNEL", "dimless",
                                                  wanda_property_types::GLOQUANT, "OC");
    }
    // COS properties
    groupname = classsort_key + "_COS";
    int n_ctr_out_specs = _database.get_int_attribute(groupname, "N_ctr_out_specs");
    nefis_uindex ctr_out_index = {1, n_ctr_out_specs, 1};
    if (n_ctr_out_specs > 0)
    {
        std::vector<std::string> spec_descr(n_ctr_out_specs);
        std::vector<std::string> spec_descr_short(n_ctr_out_specs);
        std::vector<std::string> comp_sp_inp_fld(n_ctr_out_specs);
        std::vector<std::string> unit_dim(n_ctr_out_specs);
        _database.get_string_element(groupname, "Spec_descr", ctr_out_index, 30, spec_descr);
        _database.get_string_element(groupname, "Spec_descr_short", ctr_out_index, 3, spec_descr_short);
        _database.get_string_element(groupname, "Comp_sp_inp_fld", ctr_out_index, 1, comp_sp_inp_fld);
        _database.get_string_element(groupname, "Unit_dimension", ctr_out_index, 12, unit_dim);
        for (int i = 0; i < n_ctr_out_specs; i++)
        {
            numproperties++;
            propertylist[spec_descr[i]] =
                wanda_property(numproperties, spec_descr[i], '0', comp_sp_inp_fld[i][0], "COS", unit_dim[i],
                               wanda_property_types::COS, spec_descr_short[i]);
            propertylist[spec_descr[i]].set_hos_index(i);
        }
    }
    groupname = classsort_key + "_COV";
    n_ctr_out_specs = _database.get_int_attribute(groupname, "N_ctr_out_specs");
    ctr_out_index.end = n_ctr_out_specs;
    if (n_ctr_out_specs > 0)
    {
        std::vector<std::string> spec_descr(n_ctr_out_specs);
        std::vector<std::string> spec_descr_short(n_ctr_out_specs);
        std::vector<std::string> comp_sp_inp_fld(n_ctr_out_specs);
        std::vector<std::string> unit_dim(n_ctr_out_specs);
        _database.get_string_element(groupname, "Spec_descr", ctr_out_index, 30, spec_descr);
        _database.get_string_element(groupname, "Spec_descr_short", ctr_out_index, 3, spec_descr_short);
        _database.get_string_element(groupname, "Comp_sp_inp_fld", ctr_out_index, 1, comp_sp_inp_fld);
        _database.get_string_element(groupname, "Unit_dimension", ctr_out_index, 12, unit_dim);

        for (int i = 0; i < n_ctr_out_specs; i++)
        {
            numproperties++;
            propertylist[spec_descr[i]] =
                wanda_property(numproperties, spec_descr[i], '0', comp_sp_inp_fld[i][0], "COV", unit_dim[i],
                               wanda_property_types::COV, spec_descr_short[i]);
            propertylist[spec_descr[i]].set_hos_index(i);
        }
    }
    return propertylist;
}

std::unordered_map<std::string, wanda_property> wanda_def::get_node_input_properties(std::string classsort_key)
{
    std::string groupname = classsort_key + "_NOD";
    int hydr_inp_specs = _database.get_int_attribute(groupname, "N_hydr_inp_specs");
    std::unordered_map<std::string, wanda_property> propertylist(hydr_inp_specs);
    if (hydr_inp_specs > 0)
    {
        std::vector<std::string> spec_descr(hydr_inp_specs);
        std::vector<std::string> spec_descr_short(hydr_inp_specs);
        std::vector<std::string> comp_sp_inp_fld(hydr_inp_specs);
        std::vector<std::string> unit_dim(hydr_inp_specs);
        std::vector<std::string> table_type_key(hydr_inp_specs);
        std::vector<std::vector<std::string>> lists(hydr_inp_specs, std::vector<std::string>(10));
        std::vector<int> N_toggle_fields(hydr_inp_specs);
        std::vector<float> min_value(hydr_inp_specs);
        std::vector<float> max_value(hydr_inp_specs);
        std::vector<float> default_value(hydr_inp_specs);
        std::vector<std::string> list_dependency(hydr_inp_specs);
        std::vector<int> view_list_mask(hydr_inp_specs);
        std::vector<int> view_list(hydr_inp_specs);
        std::vector<std::string> input_type_code(hydr_inp_specs);
        const nefis_uindex hydr_inp_index = {1, hydr_inp_specs, 1};
        _database.get_string_element(groupname, "Spec_descr", hydr_inp_index, 30, spec_descr);
        _database.get_string_element(groupname, "Spec_descr_short", hydr_inp_index, 3, spec_descr_short);
        _database.get_string_element(groupname, "Comp_sp_inp_fld", hydr_inp_index, 1, comp_sp_inp_fld);
        _database.get_string_element(groupname, "Unit_dimension", hydr_inp_index, 12, unit_dim);
        _database.get_string_element(groupname, "Table_type_key", hydr_inp_index, 8, table_type_key);
        _database.get_string_element(groupname, "Toggle_fields", {1, hydr_inp_specs, 1}, {1, 10, 1}, 16, lists);
        _database.get_int_element(groupname, "N_toggle_fields", {1, hydr_inp_specs, 1}, N_toggle_fields);
        _database.get_float_element(groupname, "Min_value", {1, hydr_inp_specs, 1}, min_value);
        _database.get_float_element(groupname, "Max_value", {1, hydr_inp_specs, 1}, max_value);
        _database.get_float_element(groupname, "Default_value", {1, hydr_inp_specs, 1}, default_value);
        _database.get_string_element(groupname, "List_dependency", hydr_inp_index, 30, list_dependency);
        _database.get_int_element(groupname, "View_list_mask", {1, hydr_inp_specs, 1}, view_list_mask);
        _database.get_int_element(groupname, "View_mask", {1, hydr_inp_specs, 1}, view_list);
        _database.get_string_element(groupname, "Input_type_code", hydr_inp_index, 1, input_type_code);
        for (int i = 0; i < hydr_inp_specs; i++)
        {
            propertylist[spec_descr[i]] = wanda_property(
                i, spec_descr[i], '0', comp_sp_inp_fld[i][0], "NIS", unit_dim[i], wanda_property_types::NIS,
                spec_descr_short[i], default_value[i], min_value[i], max_value[i], list_dependency[i],
                view_list_mask[i], input_type_code[i][0], view_list[i]);
            if (propertylist[spec_descr[i]].get_property_spec_inp_fld() == 'C')
            {
                std::vector<std::string> list(lists[i].begin(), lists[i].begin() + N_toggle_fields[i]);
                propertylist[spec_descr[i]].set_list(list);
            }

            if (propertylist[spec_descr[i]].get_property_spec_inp_fld() != 'R' &&
                propertylist[spec_descr[i]].get_property_spec_inp_fld() != 'I' &&
                propertylist[spec_descr[i]].get_property_spec_inp_fld() != 'C')
            {
                // setting additional settings for table
                wanda_tab_info tab_info;
                int counter = 0;
                if (comp_sp_inp_fld[i][0] == 'S')
                {
                    tab_info = string_col_info[table_type_key[i]];
                    counter = 1;
                }
                else if (comp_sp_inp_fld[i][0] == 'N')
                {
                    tab_info = num_col_info[table_type_key[i]];
                    counter = 1;
                }
                else if (comp_sp_inp_fld[i][0] == 'T')
                {
                    tab_info = table_info[table_type_key[i]];
                    counter = 2;
                }
                wanda_table &table = propertylist[spec_descr[i]].get_table();
                for (int j = 0; j < counter; j++)
                {
                    table.add_column(tab_info.description[j], tab_info.unit_dim[j], "Unrefrnc", comp_sp_inp_fld[i][0],
                                     i, j, tab_info.description[j == 0 ? counter - 1 : 0], 'C');
                }
            }
        }
    }
    return propertylist;
}

std::unordered_map<std::string, wanda_property> wanda_def::get_node_output_properties(std::string classsort_key)
{
    int numproperties = -1;
    std::string groupname = classsort_key + "_NOD";
    std::vector<std::string> node_outp_quant(16);
    int n_avail_quants = _database.get_int_attribute("GLOBAL_QUANTITIE", "N_avail_quants");
    std::vector<std::string> quantity_symbol(n_avail_quants);
    std::vector<std::string> quantity_name(n_avail_quants);
    std::vector<std::string> WDO_postfix(n_avail_quants);
    std::vector<std::string> unit_dim(n_avail_quants);
    std::unordered_map<std::string, wanda_property> propertylist;
    const nefis_uindex avail_quants_index = {1, n_avail_quants, 1};
    _database.get_string_element(groupname, "Node_outp_quant", nefis_file::single_elem_uindex, 16, node_outp_quant);
    _database.get_string_element("GLOBAL_QUANTITIE", "Quantity_symbol", avail_quants_index, 1, quantity_symbol);
    _database.get_string_element("GLOBAL_QUANTITIE", "Quantity_name", avail_quants_index, 30, quantity_name);
    _database.get_string_element("GLOBAL_QUANTITIE", "WDO_postfix", avail_quants_index, 11, WDO_postfix);
    _database.get_string_element("GLOBAL_QUANTITIE", "Unit_dimension", avail_quants_index, 12, unit_dim);
    for (size_t j = 0; j < to_up(node_outp_quant[0]).size(); ++j)
    {
        numproperties++;
        __int64 index = find(quantity_symbol.begin(), quantity_symbol.end(), node_outp_quant[0].substr(j, 1)) -
                        quantity_symbol.begin();
        if (index == quantity_symbol.size())
            throw std::out_of_range("Cannot find " + node_outp_quant[0].substr(j, 1) + " in node output quantity list");
        std::string spec_descr = (quantity_name[index]);
        propertylist[spec_descr] =
            wanda_property(numproperties, spec_descr, '0', 'G', WDO_postfix[index], unit_dim[index],
                           wanda_property_types::GLOQUANT, node_outp_quant[0].substr(j, 1));
    }

    groupname = classsort_key + "_NOS";
    int n_hydr_out_specs = _database.get_int_attribute(groupname, "N_hydr_out_specs");
    nefis_uindex hydr_out_index = {1, n_hydr_out_specs, 1};
    if (n_hydr_out_specs > 0)
    {
        std::vector<std::string> spec_descr(n_hydr_out_specs);
        std::vector<std::string> spec_descr_short(n_hydr_out_specs);
        std::vector<std::string> unit_dim_nos(n_hydr_out_specs);
        std::vector<std::string> comp_sp_inp_fld(n_hydr_out_specs);
        _database.get_string_element(groupname, "Spec_descr", hydr_out_index, 30, spec_descr);
        _database.get_string_element(groupname, "Spec_descr_short", hydr_out_index, 3, spec_descr_short);
        _database.get_string_element(groupname, "Comp_sp_inp_fld", hydr_out_index, 1, comp_sp_inp_fld);
        _database.get_string_element(groupname, "Unit_dimension", hydr_out_index, 12, unit_dim_nos);
        for (int i = 0; i < n_hydr_out_specs; i++)
        {
            numproperties++;
            propertylist[spec_descr[i]] =
                wanda_property(numproperties, spec_descr[i], '0', comp_sp_inp_fld[i][0], "NOS", unit_dim_nos[i],
                               wanda_property_types::NOS, spec_descr_short[i]);
            propertylist[spec_descr[i]].set_hos_index(i);
        }
    }
    groupname = classsort_key + "_NOV";
    n_hydr_out_specs = _database.get_int_attribute(groupname, "N_hydr_out_specs");
    hydr_out_index.end = n_hydr_out_specs;
    if (n_hydr_out_specs > 0)
    {
        std::vector<std::string> spec_descr(n_hydr_out_specs);
        std::vector<std::string> spec_descr_short(n_hydr_out_specs);
        std::vector<std::string> comp_sp_inp_fld(n_hydr_out_specs);
        std::vector<std::string> unit_dim_nov(n_hydr_out_specs);
        _database.get_string_element(groupname, "Spec_descr", hydr_out_index, 30, spec_descr);
        _database.get_string_element(groupname, "Spec_descr_short", hydr_out_index, 3, spec_descr_short);
        _database.get_string_element(groupname, "Comp_sp_inp_fld", hydr_out_index, 1, comp_sp_inp_fld);
        _database.get_string_element(groupname, "Unit_dimension", hydr_out_index, 12, unit_dim_nov);
        for (int i = 0; i < n_hydr_out_specs; i++)
        {
            numproperties++;
            propertylist[spec_descr[i]] =
                wanda_property(numproperties, spec_descr[i], '0', comp_sp_inp_fld[i][0], "NOV", unit_dim_nov[i],
                               wanda_property_types::NOV, spec_descr_short[i]);
            propertylist[spec_descr[i]].set_hos_index(i);
        }
    }
    return propertylist;
}

std::string wanda_def::get_name_prefix_phys_comp(std::string classsort_key)
{
    std::string groupname = classsort_key + "_GEN";

    std::vector<std::string> resultarray(1);
    _database.get_string_element(groupname, "Name_prefix", nefis_file::single_elem_uindex, 8, resultarray);
    return resultarray[0];
}

std::string wanda_def::get_name_prefix_phys_node(std::string classsort_key)
{
    std::string groupname = classsort_key + "_NOD";
    std::vector<std::string> resultarray(1);
    _database.get_string_element(groupname, "Name_prefix", nefis_file::single_elem_uindex, 8, resultarray);
    return resultarray[0];
}

std::string wanda_def::get_name_prefix_ctrl_comp(std::string classsort_key)
{
    std::string groupname = classsort_key + "_CTR";
    std::vector<std::string> resultarray(1);
    _database.get_string_element(groupname, "Name_prefix", nefis_file::single_elem_uindex, 8, resultarray);
    return resultarray[0];
}

std::string wanda_def::get_class_name_phys_comp(std::string classsort_key)
{
    std::string groupname = classsort_key + "_GEN";
    std::vector<std::string> resultarray(1);
    _database.get_string_element(groupname, "Class_name", nefis_file::single_elem_uindex, 8, resultarray);
    return resultarray[0];
}

std::string wanda_def::get_class_name_phys_node(std::string classsort_key)
{
    std::string groupname = classsort_key + "_NOD";
    std::vector<std::string> resultarray(1);
    _database.get_string_element(groupname, "Class_name", nefis_file::single_elem_uindex, 8, resultarray);
    return resultarray[0];
}

std::string wanda_def::get_class_name_ctrl_comp(std::string classsort_key)
{
    std::string groupname = classsort_key + "_CTR";
    std::vector<std::string> resultarray(1);
    _database.get_string_element(groupname, "Class_name", nefis_file::single_elem_uindex, 8, resultarray);
    return resultarray[0];
}

std::string wanda_def::get_type_phys_comp(std::string classsort_key)
{
    std::string groupname = classsort_key + "_GEN";
    std::vector<std::string> resultarray(1);
    _database.get_string_element(groupname, "Comp_type", nefis_file::single_elem_uindex, 8, resultarray);
    return resultarray[0];
}

std::string wanda_def::get_type_name_phys(std::string classsort_key)
{
    std::string groupname = classsort_key + "_GEN";
    std::vector<std::string> resultarray(1);
    _database.get_string_element(groupname, "Type_name", nefis_file::single_elem_uindex, 48, resultarray);
    return resultarray[0];
}

std::string wanda_def::get_type_name_node(std::string classsort_key)
{
    std::string groupname = classsort_key + "_NOD";
    std::vector<std::string> resultarray(1);
    _database.get_string_element(groupname, "Type_name", nefis_file::single_elem_uindex, 48, resultarray);
    return resultarray[0];
}

std::string wanda_def::get_type_name_ctrl(std::string classsort_key)
{
    std::string groupname = classsort_key + "_CTR";
    std::vector<std::string> resultarray(1);
    _database.get_string_element(groupname, "Type_name", nefis_file::single_elem_uindex, 48, resultarray);
    return resultarray[0];
}

std::string wanda_def::get_convert2_comp(std::string classsort_key)
{
    std::vector<std::string> convert2comp(1);
    std::string groupname;
    if (is_physical_component(classsort_key))
    {
        groupname = classsort_key + "_GEN";
    }
    if (is_control_component(classsort_key))
    {
        groupname = classsort_key + "_CTR";
    }
    if (is_node(classsort_key))
    {
        groupname = classsort_key + "_NOD";
    }
    _database.get_string_element(groupname, "Convert2key", nefis_file::single_elem_uindex, 8, convert2comp);
    return convert2comp[0];
}

std::string wanda_def::get_default_mask(std::string classsort_key)
{
    std::vector<std::string> convert2comp(1);
    std::string groupname;
    if (is_physical_component(classsort_key))
    {
        groupname = classsort_key + "_GEN";
    }
    if (is_control_component(classsort_key))
    {
        groupname = classsort_key + "_CTR";
    }
    if (is_node(classsort_key))
    {
        groupname = classsort_key + "_NOD";
    }
    _database.get_string_element(groupname, "Default_mask", nefis_file::single_elem_uindex, 10, convert2comp);
    return convert2comp[0];
}

int wanda_def::get_physical_num_com_his(std::string classsort_key)
{
    std::string groupname = classsort_key + "_HIS";
    return _database.get_int_attribute(groupname, "N_his_com");
}

int wanda_def::get_physical_num_ope_his(std::string classsort_key)
{
    std::string groupname = classsort_key + "_HIS";
    return _database.get_int_attribute(groupname, "N_his_ope");
}

bool wanda_def::is_physical_component(std::string classsort_key)
{
    bool out;
    std::unordered_set<std::string>::const_iterator value = _phys_class_sort_keys.find(classsort_key);
    if (value == _phys_class_sort_keys.end())
        out = false;
    else
        out = true;
    return out;
}

bool wanda_def::is_control_component(std::string classsort_key)
{
    bool out;
    std::unordered_set<std::string>::const_iterator value = _control_class_sort_keys.find(classsort_key);
    if (value == _control_class_sort_keys.end())
        out = false;
    else
        out = true;
    return out;
}

bool wanda_def::is_node(std::string classsort_key)
{
    std::unordered_set<std::string>::const_iterator value = _node_class_sort_keys.find(classsort_key);
    if (value == _node_class_sort_keys.end())
        return false;
    return true;
}

std::string wanda_def::to_up(std::string in) const
{
    transform(in.begin(), in.end(), in.begin(), toupper);
    std::string out = in;
    return out;
}

int wanda_def::get_key_index_array(std::vector<std::string> KeyArray, std::string key)
{
    ptrdiff_t pos = find(KeyArray.begin(), KeyArray.end(), key) - KeyArray.begin();
    if (pos >= int(KeyArray.size()))
        return -1;
    return int(pos);
}

std::vector<std::string> wanda_def::get_comp_spec_ip_fld(std::string classsort_key)
{
    std::vector<std::string> _spec_ip_fld;
    int nis;
    if (is_control_component(classsort_key))
    {
        std::string grpname = classsort_key + "_CTR";
        nis = _database.get_int_attribute(grpname, "N_ctr_specs");
        _spec_ip_fld.resize(nis);
        _database.get_string_element(grpname, "Comp_sp_inp_fld", {1, nis, 1}, 1, _spec_ip_fld);
    }
    else if (is_node(classsort_key))
    {
        std::string grpname = classsort_key + "_NOD";
        nis = _database.get_int_attribute(grpname, "N_hydr_inp_specs");
        _spec_ip_fld.resize(nis);
        _database.get_string_element(grpname, "Comp_sp_inp_fld", {1, nis, 1}, 1, _spec_ip_fld);
    }
    else
    {
        throw std::invalid_argument(classsort_key + " is not a control component or node");
    }
    for (int i = nis; i < 36; i++)
        _spec_ip_fld.push_back(" ");
    return _spec_ip_fld;
}

std::vector<int> wanda_def::get_max_in_chan(std::string classsort_key)
{
    std::string grpname = classsort_key + "_CTR";
    std::vector<int> max_input_chan(16);
    _database.get_int_element(grpname, "Chanl_in_maxsign", nefis_file::single_elem_uindex, max_input_chan);
    return max_input_chan;
}

std::vector<int> wanda_def::get_min_in_chan(std::string classsort_key)
{
    std::string grpname = classsort_key + "_CTR";
    std::vector<int> min_input_chan(16);
    _database.get_int_element(grpname, "Chanl_in_minsign", nefis_file::single_elem_uindex, min_input_chan);
    return min_input_chan;
}

std::vector<std::string> wanda_def::get_in_chan_type(std::string classsort_key)
{
    std::string grpname = classsort_key + "_CTR";
    std::vector<std::string> input_chan_type(16);
    _database.get_string_element(grpname, "Chanl_in_type", nefis_file::single_elem_uindex, 8, input_chan_type);
    return input_chan_type;
}

std::vector<std::string> wanda_def::get_out_chan_type(std::string classsort_key)
{
    std::string grpname = classsort_key + "_CTR";
    std::vector<std::string> input_chan_type(16);
    _database.get_string_element(grpname, "Chanl_out_type", nefis_file::single_elem_uindex, 8, input_chan_type);
    return input_chan_type;
}

std::string wanda_def::get_ctrl_input_type(std::string classsort_key)
{
    if (ctrl_input_type.find(classsort_key) == ctrl_input_type.end())
    {
        std::string grpname = classsort_key + "_GEN";
        std::vector<std::string> input_type(1);
        _database.get_string_element(grpname, "Ctrl_input_type", nefis_file::single_elem_uindex, 8, input_type);
        ctrl_input_type.emplace(classsort_key, input_type[0]);
    }
    return ctrl_input_type[classsort_key];
}

std::string wanda_def::get_data_path() const
{
    return _data_path;
}

int wanda_def::get_num_con_points(std::string classsort_key)
{
    std::string grpname = classsort_key + "_GEN";
    std::vector<int> num_con_points(1);
    _database.get_int_element(grpname, "H_node_count", nefis_file::single_elem_uindex, num_con_points);
    return num_con_points[0];
}

std::string wanda_def::get_quant_name(char symbol)
{
    int N_avail_quants = _database.get_int_attribute("GLOBAL_QUANTITIE", "N_avail_quants");
    std::vector<std::string> quantsymbol(N_avail_quants);
    std::vector<std::string> quantname(N_avail_quants);
    _database.get_string_element("GLOBAL_QUANTITIE", "Quantity_symbol", {1, N_avail_quants, 1}, 1, quantsymbol);
    _database.get_string_element("GLOBAL_QUANTITIE", "Quantity_name", {1, N_avail_quants, 1}, 30, quantname);
    std::string _symbol = std::to_string(symbol);
    __int64 index = find(quantsymbol.begin(), quantsymbol.end(), _symbol) - quantsymbol.begin();
    return quantname[index];
}

char wanda_def::get_quant_symbol(std::string quant_name)
{
    int N_avail_quants = _database.get_int_attribute("GLOBAL_QUANTITIE", "N_avail_quants");
    std::vector<std::string> quantsymbol(N_avail_quants);
    std::vector<std::string> quantname(N_avail_quants);
    _database.get_string_element("GLOBAL_QUANTITIE", "Quantity_symbol", {1, N_avail_quants, 1}, 1, quantsymbol);
    _database.get_string_element("GLOBAL_QUANTITIE", "Quantity_name", {1, N_avail_quants, 1}, 30, quantname);
    __int64 index = find(quantname.begin(), quantname.end(), quant_name) - quantname.begin();
    return quantsymbol[index][0];
}

std::vector<std::string> wanda_def::get_list_quant_names(std::string classsort_key)
{
    std::vector<std::string> glob_outp_quant(1);
    if (is_physical_component(classsort_key))
    {
        std::string groupname = classsort_key + "_GEN";
        _database.get_string_element(groupname, "Glob_outp_quant", nefis_file::single_elem_uindex, 16, glob_outp_quant);
    }
    else if (is_node(classsort_key))
    {
        std::string groupname = classsort_key + "_NOD";
        _database.get_string_element(groupname, "Node_outp_quant", nefis_file::single_elem_uindex, 16, glob_outp_quant);
    }

    int n_avail_quants = _database.get_int_attribute("GLOBAL_QUANTITIE", "N_avail_quants");
    std::vector<std::string> quantsymbol(n_avail_quants);
    std::vector<std::string> quantname(n_avail_quants);
    std::vector<std::string> return_list;

    _database.get_string_element("GLOBAL_QUANTITIE", "Quantity_symbol", {1, n_avail_quants, 1}, 1, quantsymbol);
    _database.get_string_element("GLOBAL_QUANTITIE", "Quantity_name", {1, n_avail_quants, 1}, 30, quantname);
    for (int j = 0; j < n_avail_quants; j++)
    {
        if (glob_outp_quant[0].find(quantsymbol[j]) != std::string::npos)
        {
            // to avoid that sensor can measure Composition
            if (quantname[j] != "Composition")
            {
                return_list.push_back(quantname[j]);
            }
        }
    }
    return return_list;
}

int wanda_def::get_number_of_con_points(std::string classsort_key)
{
    std::string groupname = classsort_key + "_GEN";
    std::vector<int> h_node_counts(1);
    _database.get_int_element(groupname, "H_node_count", nefis_file::single_elem_uindex, h_node_counts);
    return h_node_counts[0];
}

bool wanda_def::get_controlable(std::string classsort_key)
{
    std::string groupname = classsort_key + "_GEN";
    std::vector<int> comp_contrl_code(1);
    _database.get_int_element(groupname, "Comp_contrl_code", nefis_file::single_elem_uindex, comp_contrl_code);
    return (comp_contrl_code[0] == -1);
}

float wanda_def::get_unit_factor(std::string unit_dim, std::string unit_descr)
{
    if (unit_list.find(unit_dim) == unit_list.end())
    {
        throw std::invalid_argument(unit_dim + " does not exist in Wanda");
    }
    if (unit_list[unit_dim].find(unit_descr) == unit_list[unit_dim].end())
    {
        throw std::invalid_argument(unit_descr + " does not exist in " + unit_dim);
    }
    return unit_list[unit_dim][unit_descr];
}

wanda_component *wanda_def::get_component(std::string classsort_key)
{
    if (_components_list.find(classsort_key) == _components_list.end())
    {
        std::string name_pre_fix;
        wanda_type type;
        int num_com_spec = 0;
        int num_oper_spec = 0;
        int num_hcs = 0;
        int number_of_connnect_points = 0;
        bool controlable = false;
        std::vector<std::string> core_quants;
        std::unordered_map<std::string, wanda_property> props = get_properties(classsort_key);
        std::string def_mask = get_default_mask(classsort_key);
        std::string conv2comp = get_convert2_comp(classsort_key);
        std::string h_ctrl_input;
        std::string type_name;
        std::string typecomp;
        int max_input_channels = 0;
        int min_input_channels = 0;
        if (is_physical_component(classsort_key))
        {
            std::vector<int> inputs = get_num_input_props(classsort_key);
            num_com_spec = inputs[0];
            num_oper_spec = inputs[1];
            num_hcs = inputs[2];
            name_pre_fix = get_name_prefix_phys_comp(classsort_key);
            type = wanda_type::physical;
            number_of_connnect_points = get_number_of_con_points(classsort_key);
            controlable = get_controlable(classsort_key);
            core_quants = get_core_quants(classsort_key);
            h_ctrl_input = get_ctrl_input_type(classsort_key);
            type_name = get_type_name_phys(classsort_key);
            typecomp = get_type_phys_comp(classsort_key);
        }
        else
        {
            name_pre_fix = get_name_prefix_ctrl_comp(classsort_key);
            type = wanda_type::control;
            type_name = get_type_name_ctrl(classsort_key);

            if (classsort_key == "SENSOR")
            {
                number_of_connnect_points = 1;
            }
        }
        _components_list.emplace(
            classsort_key, wanda_component(classsort_key, name_pre_fix, type, typecomp, num_com_spec, num_oper_spec,
                                           num_hcs, number_of_connnect_points, controlable, core_quants, props,
                                           h_ctrl_input, type_name, def_mask, conv2comp, this));
    }
    return &_components_list[classsort_key];
}

wanda_node *wanda_def::get_node(std::string classsort_key)
{
    if (_node_list.find(classsort_key) == _node_list.end())
    {
        std::string def_mask = get_default_mask(classsort_key);
        std::string conv2comp = get_convert2_comp(classsort_key);
        std::string type_name = get_type_name_node(classsort_key);
        std::string name_pre_fix = get_name_prefix_phys_node(classsort_key);
        _node_list.emplace(classsort_key, wanda_node(classsort_key, wanda_type::node, name_pre_fix, type_name, def_mask,
                                                     conv2comp, this));
    }
    return &_node_list[classsort_key];
}

std::unordered_map<std::string, wanda_property> wanda_def::get_properties(std::string classsort_key)
{
    if (is_physical_component(classsort_key))
    {
        if (phys_props.find(classsort_key) == phys_props.end())
        {
            std::unordered_map<std::string, wanda_property> props = get_physical_input_properties(classsort_key);
            std::unordered_map<std::string, wanda_property> calc_prop = get_physical_calc_properties(classsort_key);
            std::unordered_map<std::string, wanda_property> out_prop = get_physical_output_properties(classsort_key);
            props.insert(calc_prop.begin(), calc_prop.end());
            props.insert(out_prop.begin(), out_prop.end());
            phys_props.emplace(classsort_key, props);
        }
        return phys_props[classsort_key];
    }
    if (is_control_component(classsort_key))
    {
        std::unordered_map<std::string, wanda_property> props = get_control_input_properties(classsort_key);
        std::unordered_map<std::string, wanda_property> out_prop = get_control_output_properties(classsort_key);
        props.insert(out_prop.begin(), out_prop.end());
        return props;
    }
    if (is_node(classsort_key))
    {
        if (node_props.find(classsort_key) == node_props.end())
        {
            std::unordered_map<std::string, wanda_property> props = get_node_input_properties(classsort_key);
            std::unordered_map<std::string, wanda_property> out_prop = get_node_output_properties(classsort_key);
            props.insert(out_prop.begin(), out_prop.end());
            node_props.emplace(classsort_key, props);
        }
        return node_props[classsort_key];
    }
    throw std::invalid_argument(classsort_key + " does not exist in Wandadef");
}

void wanda_def::load_unit_list()
{
    int size = _database.get_maxdim_index("UNIT_TABLE");
    std::vector<std::string> unit_dimension(size);
    std::vector<std::string> unit_desc(size);
    std::vector<float> unit_factor(size);

    _database.get_string_element("UNIT_TABLE", "Unit_dimension", {1, size, 1}, 12, unit_dimension);
    _database.get_string_element("UNIT_TABLE", "Unit_descr", {1, size, 1}, 16, unit_desc);
    _database.get_float_element("UNIT_TABLE", "Unit_factor", {1, size, 1}, unit_factor);

    for (int i = 0; i < size; i++)
    {
        if (unit_list.find(unit_dimension[i]) != unit_list.end())
        {
            unit_list[unit_dimension[i]].emplace(unit_desc[i], unit_factor[i]);
        }
        else
        {
            std::unordered_map<std::string, float> map;
            map.emplace(unit_desc[i], unit_factor[i]);
            unit_list.emplace(unit_dimension[i], map);
        }
    }
}

std::string wanda_def::get_wanda_version() const
{
    return wanda_version_.to_string();
}

void wanda_def::load_version_number()
{
    std::vector<std::string> wanda_version(1);
    _database.get_string_element("WANDA", "Wanda_version", nefis_file::single_elem_uindex, 0, wanda_version);
    wanda_version_ = wanda_helper_functions::wanda_version_number(wanda_version[0]);
}