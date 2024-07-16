#include "calc_hcs.h"

#include <algorithm>
#include <array>
#include <btps.h>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include <unordered_map>
#include <vector>

#include <lcencdec.h>
#include "deltares_helper_functions.h"
#include "globvar.h"
#include "mode_and_options.h"
#include "nefis_file.h"
#include "wandacomponent.h"
#include "wandadef.h"
#include "wandamodel.h"
#include "wandanode.h"
#include "wandaproperty.h"
#include "wandasigline.h"
#include "deltares_helper_functions.h"

#ifdef _WIN32
#include <windows.h>
#include <winver.h>
#endif

// Dauth defines C++ functions in global scope, prefix function calls with ::
// for clarity

// the following vars are used in license auth calls.
const char *lic_feat_WANDA_SYSTEM = "WANDA_MODEL_ENGINE";
const char *lic_feat_version = "4.0";

bool FileExists(std::string filename)
{ // Check if a file exists returns true when the
  // file exists and false when it does not exists.
    return std::filesystem::exists(filename);
}

inline std::string int2string(int n, int len)
{
    std::string result(len--, '0');
    for (int val = (n < 0) ? -n : n; len >= 0 && val != 0; --len, val /= 10)
        result[len] = '0' + val % 10;
    if (len >= 0 && n < 0)
        result[0] = '-';
    return result;
}

bool wanda_model::is_modified() const
{
    if (_modified)
        return true;
    auto check_modified = [](const auto &item) { return item.second.is_modified(); };
    if (std::any_of(phys_components.cbegin(), phys_components.cend(), check_modified))
    {
        return true;
    }
    if (std::any_of(ctrl_components.cbegin(), ctrl_components.cend(), check_modified))
    {
        return true;
    }
    if (std::any_of(phys_nodes.cbegin(), phys_nodes.cend(), check_modified))
    {
        return true;
    }
    if (std::any_of(signal_lines.cbegin(), signal_lines.cend(), check_modified))
    {
        return true;
    }
    return false;
}

std::string wanda_model::get_new_table_key(std::string description, wanda_table &table)
{
    std::string table_key = get_unique_key(&table_metainfo_cache, table.get_table_type(description), last_key);
    tabcol_meta_record tab_info;
    if (table.get_table_type(description) == 'T')
    {
        tab_info.index = index_table;
        index_table++;
    }
    else if ((table.get_table_type(description) == 'N'))
    {
        tab_info.index = index_num_col;
        index_num_col++;
    }
    else if ((table.get_table_type(description) == 'S'))
    {
        tab_info.index = index_string_col;
        index_string_col++;
    }
    tab_info.table_key_next = unref;
    tab_info.size = 0;
    table_metainfo_cache.emplace(table_key, tab_info);
    return table_key;
}

void wanda_model::save_table(wanda_table &table)
{
    for (std::string description : table.get_descriptions())
    {
        // create new keys if required.
        if (table.has_data(description))
        {
            std::string table_key = table.get_key(description);
            if (!(table.get_col_num(description) == 0 && table.get_table_type(description) == 'T'))
            {
                if ((table.get_key(description) == unref || table.get_key(description) == ""))
                {
                    table_key = get_new_table_key(description, table);
                    table.set_key(description, table_key);
                    for (auto rel_descr : table.get_related_prop(description))
                    {
                        if (!rel_descr.empty())
                        {
                            table.set_key(rel_descr, table_key);
                        }
                    }
                }
            }

            if (table.get_table_type(description) == 'T' && table.get_col_num(description) == 1)
            {
                tab_nefis_info nefis_details;
                nefis_details.el_description = "Table_descr";
                nefis_details.el_size = "Table_size";
                nefis_details.el_table_key_next = "Table_key_next";
                nefis_details.el_table_type_key = "Table_type_key";
                nefis_details.el_unit_dim = "Table_unit_dim";
                nefis_details.group_name = "TABLES";
                nefis_details.el_values = "Table_values";

                std::vector<float> data1;
                std::vector<int> size_tab;
                for (auto rel_descr : table.get_related_prop(description))
                {
                    data1 = table.get_float_column(rel_descr);
                    size_tab.push_back(static_cast<int>(data1.size()));
                }
                std::vector<float> data2 = table.get_float_column(description);

                std::vector<std::vector<std::string>> unit_desscription(1);
                std::vector<std::vector<std::string>> descriptions(1);
                descriptions[0].push_back(table.get_related_prop(description)[0]);
                descriptions[0].push_back(description);

                unit_desscription[0].push_back(table.get_unit(table.get_related_prop(description)[0]));
                unit_desscription[0].push_back(table.get_unit(description));

                int num_groups = ceil(data1.size() / 100);

                for (size_t group_counter = 0; group_counter <= num_groups; group_counter++)
                {
                    int index = table_metainfo_cache.at(table_key).index;
                    std::vector<float> data;
                    if (data1.size() < (group_counter + 1) * 100)
                    {
                        data.insert(data.end(), data1.begin() + group_counter * 100, data1.end());
                        while (data.size() < 100)
                        {
                            data.push_back(0.0);
                        }
                        data.insert(data.end(), data2.begin() + group_counter * 100, data2.end());
                        while (data.size() < 200)
                        {
                            data.push_back(0.0);
                        }
                        if (num_groups >= 1)
                        {
                            if (group_counter != 0)
                            {
                                if (table_metainfo_cache.at(table_key).table_key_next == unref ||
                                    table_metainfo_cache.at(table_key).table_key_next == "")
                                {
                                    std::string table_key_new = get_new_table_key(description, table);
                                    table_metainfo_cache.at(table_key).table_key_next = table_key_new;
                                    std::vector<std::string> tab_key_next_vec;
                                    tab_key_next_vec.push_back(table_key_new);
                                    wanda_input_file.write_string_elements(
                                        nefis_details.group_name, nefis_details.el_table_key_next,
                                        {index + 1, index + 1, 1}, 8, tab_key_next_vec);
                                    table_key = table_key_new;
                                }
                                else
                                {
                                    table_key = table_metainfo_cache.at(table_key).table_key_next;
                                }
                            }
                        }
                    }
                    else
                    {
                        data.insert(data.end(), data1.begin() + group_counter * 100,
                                    data1.begin() + (group_counter + 1) * 100);
                        data.insert(data.end(), data2.begin() + group_counter * 100,
                                    data2.begin() + (group_counter + 1) * 100);
                        if (group_counter != 0)
                        {
                            if (table_metainfo_cache.at(table_key).table_key_next == unref ||
                                table_metainfo_cache.at(table_key).table_key_next == "")
                            {
                                std::string table_key_new = get_new_table_key(description, table);
                                table_metainfo_cache.at(table_key).table_key_next = table_key_new;
                                std::vector<std::string> tab_key_next_vec;
                                tab_key_next_vec.push_back(table_key_new);
                                wanda_input_file.write_string_elements(nefis_details.group_name,
                                                                       nefis_details.el_table_key_next,
                                                                       {index + 1, index + 1, 1}, 8, tab_key_next_vec);
                                table_key = table_key_new;
                            }
                            else
                            {
                                table_key = table_metainfo_cache.at(table_key).table_key_next;
                            }
                        }
                    }
                    index = table_metainfo_cache.at(table_key).index;
                    wanda_input_file.write_float_elements(nefis_details.group_name, nefis_details.el_values,
                                                          {index + 1, index + 1, 1}, data);
                    wanda_input_file.write_int_elements(nefis_details.group_name, "Table_size",
                                                        {index + 1, index + 1, 1}, size_tab);
                    wanda_input_file.write_string_elements(nefis_details.group_name, nefis_details.el_description,
                                                           {index + 1, index + 1, 1}, {1, 2, 1}, 0, descriptions);
                    wanda_input_file.write_string_elements(nefis_details.group_name, nefis_details.el_unit_dim,
                                                           {index + 1, index + 1, 1}, {1, 2, 1}, 12, unit_desscription);

                    std::vector<std::string> tab_key_vec;
                    tab_key_vec.push_back(table_key);
                    std::vector<std::string> tab_key_next_vec;
                    tab_key_next_vec.push_back(table_metainfo_cache.at(table_key).table_key_next);

                    wanda_input_file.write_string_elements("TABLES", "Table_key", {index + 1, index + 1, 1}, 8,
                                                           tab_key_vec);

                    wanda_input_file.write_string_elements("TABLES", "Table_name", {index + 1, index + 1, 1}, 8,
                                                           tab_key_vec);
                    std::vector<std::string> Table_type_key;
                    Table_type_key.push_back(unref);
                    wanda_input_file.write_string_elements(nefis_details.group_name, nefis_details.el_table_type_key,
                                                           {index + 1, index + 1, 1}, 8, Table_type_key);

                    wanda_input_file.write_string_elements(nefis_details.group_name, nefis_details.el_table_key_next,
                                                           {index + 1, index + 1, 1}, 8, tab_key_next_vec);
                }
            }
            else if (table.get_table_type(description) == 'N')
            {
                std::vector<std::string> descriptions;
                descriptions.push_back(description);

                std::vector<float> data1 = table.get_float_column(description);
                std::vector<int> size_tab;
                size_tab.push_back(data1.size());
                int num_groups = ceil(data1.size() / 100);

                for (size_t group_counter = 0; group_counter <= num_groups; group_counter++)
                {
                    int index = table_metainfo_cache.at(table_key).index;
                    std::vector<float> data;
                    std::vector<float> data_in;
                    if (data1.size() < (group_counter + 1) * 100)
                    {
                        data.insert(data.end(), data1.begin() + group_counter * 100, data1.end());
                        while (data.size() < 100)
                        {
                            data.push_back(0.0);
                        }
                        if (num_groups >= 1)
                        {
                            if (group_counter != 0)
                            {
                                if (table_metainfo_cache.at(table_key).table_key_next == unref ||
                                    table_metainfo_cache.at(table_key).table_key_next == "")
                                {
                                    std::string table_key_new =
                                        get_unique_key(&table_metainfo_cache, table.get_table_type(description),
                                                       last_key); // get_unique_key(table_metainfo_cache,
                                                                  // table.get_table_type(description),
                                                                  // last_key);
                                    table_metainfo_cache.at(table_key).table_key_next = table_key_new;

                                    std::vector<std::string> tab_key_next_vec;
                                    tab_key_next_vec.push_back(table_key_new);
                                    wanda_input_file.write_string_elements("NUMCOLUMNS", "Column_key_next",
                                                                           {index + 1, index + 1, 1}, 8,
                                                                           tab_key_next_vec);

                                    table_key = table_key_new;
                                    tabcol_meta_record tab_info;
                                    tab_info.index = index_num_col;
                                    tab_info.table_key_next = unref;
                                    tab_info.size = 0;
                                    table_metainfo_cache.emplace(table_key, tab_info);
                                    index_num_col++;
                                    // table.set_key(description, table_key);
                                }
                            }
                        }
                    }
                    else
                    {
                        data.insert(data.end(), data1.begin() + group_counter * 100,
                                    data1.begin() + (group_counter + 1) * 100);
                        if (group_counter != 0)
                        {
                            if (table_metainfo_cache.at(table_key).table_key_next == unref ||
                                table_metainfo_cache.at(table_key).table_key_next == "")
                            {
                                std::string table_key_new =
                                    get_unique_key(&table_metainfo_cache, table.get_table_type(description),
                                                   last_key); // get_unique_key(table_metainfo_cache,
                                                              // table.get_table_type(description), last_key);
                                table_metainfo_cache.at(table_key).table_key_next = table_key_new;

                                std::vector<std::string> tab_key_next_vec;
                                tab_key_next_vec.push_back(table_key_new);
                                wanda_input_file.write_string_elements("NUMCOLUMNS", "Column_key_next",
                                                                       {index + 1, index + 1, 1}, 8, tab_key_next_vec);
                                table_key = table_key_new;
                                tabcol_meta_record tab_info;
                                tab_info.index = index_num_col;
                                tab_info.table_key_next = unref;
                                tab_info.size = 0;
                                table_metainfo_cache.emplace(table_key, tab_info);
                                index_num_col++;
                                // table.set_key(description, table_key);
                            }
                        }
                    }
                    index = table_metainfo_cache.at(table_key).index;
                    wanda_input_file.write_float_elements("NUMCOLUMNS", "Column_numval", {index + 1, index + 1, 1},
                                                          data);
                    wanda_input_file.write_int_elements("NUMCOLUMNS", "Column_size", {index + 1, index + 1, 1},
                                                        size_tab);
                    std::vector<std::string> tab_key_vec;
                    tab_key_vec.push_back(table_key);
                    std::vector<std::string> tab_key_next_vec;
                    tab_key_next_vec.push_back(table_metainfo_cache.at(table_key).table_key_next);
                    wanda_input_file.write_string_elements("NUMCOLUMNS", "Column_key", {index + 1, index + 1, 1}, 8,
                                                           tab_key_vec);
                    wanda_input_file.write_string_elements("NUMCOLUMNS", "Column_key_next", {index + 1, index + 1, 1},
                                                           8, tab_key_next_vec);
                    wanda_input_file.write_string_elements("NUMCOLUMNS", "Column_descr", {index + 1, index + 1, 1}, 0,
                                                           descriptions);
                }
            }
            else if (table.get_table_type(description) == 'S')
            {
                std::vector<std::string> descriptions;
                descriptions.push_back(description);
                std::string table_key = table.get_key(description);
                if ((table.get_key(description) == unref || table.get_key(description) == ""))
                {
                    table_key = get_unique_key(&table_metainfo_cache, table.get_table_type(description), last_key);
                    tabcol_meta_record tab_info;
                    tab_info.index = index_string_col;
                    tab_info.table_key_next = unref;
                    tab_info.size = 0;
                    table_metainfo_cache.emplace(table_key, tab_info);
                    index_string_col++;
                    table.set_key(description, table_key);
                }
                std::vector<std::string> data = table.get_string_column(description);
                std::vector<int> size_tab;
                size_tab.push_back(data.size());
                size_t num_groups = ceil(data.size() / 100);
                for (size_t i = 0; i <= num_groups; i++)
                {
                    int index = table_metainfo_cache.at(table_key).index;

                    std::vector<float> data_in;
                    if (data.size() < (i + 1) * 100)
                    {
                        data.insert(data.end(), data.begin() + i * 100, data.end());
                        while (data.size() < 100)
                        {
                            data.push_back("");
                        }
                    }
                    else
                    {
                        data.insert(data.end(), data.begin() + i * 100, data.begin() + (i + 1) * 100);
                        if (table_metainfo_cache.at(table_key).table_key_next == unref ||
                            table_metainfo_cache.at(table_key).table_key_next == "")
                        {
                            std::string table_key_new =
                                get_unique_key(&table_metainfo_cache, table.get_table_type(description), last_key);
                            table_metainfo_cache.at(table_key).table_key_next = table_key_new;
                            table_key = table_key_new;
                            tabcol_meta_record tab_info;
                            tab_info.index = index_string_col;
                            tab_info.table_key_next = unref;
                            tab_info.size = 0;
                            table_metainfo_cache.emplace(table_key, tab_info);
                            index_string_col++;
                        }
                    }
                    wanda_input_file.write_string_elements("CHARCOLUMNS", "Column_charval", {index + 1, index + 1, 1},
                                                           60, data);
                    wanda_input_file.write_int_elements("CHARCOLUMNS", "Column_size", {index + 1, index + 1, 1},
                                                        size_tab);
                    std::vector<std::string> tab_key_vec;
                    tab_key_vec.push_back(table_key);
                    std::vector<std::string> tab_key_next_vec;
                    tab_key_next_vec.push_back(table_metainfo_cache.at(table_key).table_key_next);
                    wanda_input_file.write_string_elements("CHARCOLUMNS", "Column_key", {index + 1, index + 1, 1}, 8,
                                                           tab_key_vec);
                    wanda_input_file.write_string_elements("CHARCOLUMNS", "Column_key_next", {index + 1, index + 1, 1},
                                                           8, tab_key_next_vec);
                    wanda_input_file.write_string_elements("CHARCOLUMNS", "Column_descr", {index + 1, index + 1, 1}, 0,
                                                           descriptions);
                }
            }
        }
        // no data set key to unreferenced and if there is a key delete the key and
        // empty the memory
        if (table.get_key(description) != unref && !table.get_key(description).empty())
        {
            std::vector<std::string> free;
            free.emplace_back("Free");
            if (table.get_table_type(description) == 'T')
            {
                auto rel_prop = table.get_related_prop(description);
                bool has_data = table.has_data(description);
                for (auto descr : rel_prop)
                {
                    if (table.has_data(descr))
                    {
                        has_data = true;
                    }
                }
                if (!has_data)
                {
                    int index = table_metainfo_cache.at(table.get_key(description)).index;
                    wanda_input_file.write_string_elements("TABLES", "Table_key", {index + 1, index + 1, 1}, 8, free);
                    table_metainfo_cache.erase(table.get_key(description));
                    for (auto descr : rel_prop)
                    {
                        table.set_key(descr, unref);
                    }
                    table.set_key(description, unref);
                }
            }
            else if (table.get_table_type(description) == 'N')
            {
                if (!table.has_data(description))
                {
                    int index = table_metainfo_cache.at(table.get_key(description)).index;
                    wanda_input_file.write_string_elements("NUMCOLUMNS", "Column_key", {index + 1, index + 1, 1}, 8,
                                                           free);
                    table_metainfo_cache.erase(table.get_key(description));
                    table.set_key(description, unref);
                }
            }
            else if (table.get_table_type(description) == 'S')
            {
                if (!table.has_data(description))
                {
                    int index = table_metainfo_cache.at(table.get_key(description)).index;
                    wanda_input_file.write_string_elements("CHARCOLUMNS", "Column_key", {index + 1, index + 1, 1}, 8,
                                                           free);
                    table_metainfo_cache.erase(table.get_key(description));
                    table.set_key(description, unref);
                }
            }
        }
    }
    int size = wanda_input_file.get_maxdim_index("TABLES");
    wanda_input_file.set_int_attribute("TABLES", "N_group", size);
    size = wanda_input_file.get_maxdim_index("NUMCOLUMNS");
    wanda_input_file.set_int_attribute("NUMCOLUMNS", "N_group", size);
    size = wanda_input_file.get_maxdim_index("CHARCOLUMNS");
    wanda_input_file.set_int_attribute("CHARCOLUMNS", "N_group", size);
    table.set_modified(false);
}

wanda_model::wanda_model(const std::string &casefile, const std::string &Wandadir, bool upgrade_model)
//    : _object_hash(std::hash<std::string>{}(_object_name))
{

#ifdef DEBUG
    std::cout << "Wanda case file: " << casefile << '\n';
    std::cout << "Wanda bin directory: " << Wandadir << '\n';
#endif

    if (!FileExists(Wandadir + "wandadef.dat"))
    {
#ifdef DEBUG
        std::cout << Wandadir + " Not a valid Wanda bin directory,  wandadef.dat not found";
#endif
        throw std::invalid_argument("Not a valid Wanda bin directory,  wandadef.dat not found: " + Wandadir +
                                    "wandadef.dat");
    }
    wanda_bin = Wandadir;
    if (wanda_bin.back() != '\\')
    {
        wanda_bin.append("\\");
    }

    try
    {
        component_definition = new wanda_def(wanda_bin);
    }
    catch (std::runtime_error error)
    {
        std::cout << error.what();
    }
    initialize(casefile, upgrade_model);
}

wanda_model::~wanda_model()
{
    ::checkin(lic_feat_WANDA_SYSTEM);
    ::cleanup(); // cleanup license auth library
    close();
    delete component_definition;
}

std::tuple<bool,bool> wanda_model::check_wanda_version()
{

    //return std::make_tuple(version_def > version_wdi, version_wdi > version_def);
    return std::make_tuple(true, true);
}

void wanda_model::load_wanda_version()
{
    std::vector<std::string> resarray(1);
    wanda_input_file.get_string_element("WANDA", "Wanda_version", nefis_file::single_elem_uindex, 0, resarray);
    version_number = wanda_helper_functions::wanda_version_number(resarray[0]);
}

void wanda_model::calc_hsc(wanda_component& component)
{
    auto const component_dll = wanda_component_dll::get_instance(wanda_bin);
    component_dll->calc_hydraulic_spec_component(component, get_globvar_hcs());
}

void wanda_model::initialize(std::string wdifile, bool upgrade_model_in)
{
    if (!FileExists(wdifile))
    {
        new_wanda_case(wdifile);
        return;
    }
    std::string _wdofile = wdifile.substr(0, wdifile.size() - 3) + "wdo";
    this->wanda_input_file = nefis_file(wdifile);
    this->wanda_output_file = nefis_file(_wdofile);
    wanda_input_file.open();
    load_wanda_version();
    /*
     *Disused since for now this is not workign as it should and first the version numbering in UI, Core and API should be synchronized.
     *Then this check can be activated.
    const auto [upgrade_needed, WDI_newer_version] = check_wanda_version();
    if (WDI_newer_version)
    {
        throw std::runtime_error("WDI is newer then wandadef cannot load model");
    }
    if (upgrade_needed)
    {
        if (upgrade_model_in)
        {
            upgrade_model();
        }
        else
        {
            throw std::runtime_error("Wandedef is newer then wdi, please upgrade model by setting upgrade model to True");
        }
    }
    */
    std::vector<int> Next_seq_nr(1);
    wanda_input_file.get_int_element("CASE_INFORMATION", "Next_seq_nr", nefis_file::single_elem_uindex, Next_seq_nr);
    max_num_of_species = component_definition->get_max_num_species();
    last_key = Next_seq_nr[0]--;
    // read_general_items();
    read_physical_comp();
    read_nodes();
    read_control_comp();
    read_signal_lines();
    reload_input();
    if (FileExists(_wdofile))
    {
        wanda_output_file.open();
        int N_timesteps = wanda_output_file.get_int_attribute("OUTPUT_TIME", "N_timesteps");
        if (N_timesteps >= 1)
        {
#ifdef DEBUG
            std::cout << "Reading " << N_timesteps << "timesteps." << '\n';
#endif
            reload_component_indices();
            load_steady_messages();
            load_unsteady_messages();
            //  reload_output();
        }
    }
    reset_modified();
    initialized = true;
}

void wanda_model::close()
{
    if (wanda_input_file.is_open())
        wanda_input_file.close();
    if (wanda_output_file.is_open())
        wanda_output_file.close();
    // free memory
    num_cols_loaded = false;
    tables_loaded = false;
    table_metainfo_cache.clear();
    table_metainfo_cache.clear();
    string_col_loaded = false;
    output_quantity_cache.clear();
}


void wanda_model::save_model_input()
{
    // check which license is required (liquid, heat, mst, control)
    std::unordered_map<char, std::string> pos_lic_features;
    pos_lic_features['L'] = "Liquid";
    pos_lic_features['H'] = "Heat";
    pos_lic_features['M'] = "MST";
    pos_lic_features['Z'] = "Locks";
    pos_lic_features['P'] = "Prototypes";
    pos_lic_features['C'] = "Control";

    // creatin list of all features, whch are in the model
    std::set<std::string> lic_feature;
    for (auto &component : phys_components)
    {
        if (pos_lic_features.find(component.second.get_default_mask()[0]) != pos_lic_features.end())
        {
            if (lic_feature.find(pos_lic_features[component.second.get_default_mask()[0]]) == lic_feature.end())
            {
                lic_feature.insert(pos_lic_features[component.second.get_default_mask()[0]]);
            }
        }
    }
    if (ctrl_components.size() != 0)
    {
        lic_feature.insert(pos_lic_features['C']);
    }

    // check if there is a license available before saving the model.
    std::string lic_path = wanda_bin + "wanda4.lic";
    bool lic_init_ok = ::initialize(false, false, lic_path.c_str());
    bool lic_ckout = ::checkout(lic_feat_WANDA_SYSTEM, lic_feat_version);
    if (!lic_ckout)
    {
        ::checkin(lic_feat_WANDA_SYSTEM);
        ::cleanup();
        lic_init_ok = ::initialize(false, false, "");
        if (!lic_init_ok)
            throw std::runtime_error("Error during license initialisation: " + std::string(::getErrors()));
        lic_ckout = ::checkout(lic_feat_WANDA_SYSTEM, lic_feat_version);
        if (!lic_ckout)
        {
            throw std::runtime_error("Error during license checkout: " + std::string(::getErrors()));
        }
    }

    // delete WDO file
    if (wanda_output_file.is_open())
        wanda_output_file.close();
    remove(wanda_output_file.get_filename().c_str());
    bool remove_wdx = false;
    if (!initialized)
        return;

    if (!wanda_input_file.is_open())
        wanda_input_file.open();
    std::vector<std::string> caseinfo(1);
    caseinfo[0] = "WandaAPI";
    wanda_input_file.write_string_elements("CASE_INFORMATION", "Case", nefis_file::single_elem_uindex, 8, caseinfo);
    re_calculate_hcs();
    save_unit_system();
    save_mode_and_options();
    save_glob_vars();

    // delete components
    std::vector<std::string> free(1);
    free[0] = "Free";
    int nel_nodes = wanda_input_file.get_maxdim_index("H_NODES");
    std::vector<std::string> nodes(nel_nodes);
    if (nel_nodes != 0)
    {
        wanda_input_file.get_string_element("H_NODES", "H_node_key", {1, nel_nodes, 1}, 8, nodes);
    }
    int nel_comp = wanda_input_file.get_maxdim_index("H_COMPONENTS");
    std::vector<std::string> comp_key(nel_comp);
    if (nel_comp != 0)
    {
        wanda_input_file.get_string_element("H_COMPONENTS", "H_comp_key", {1, nel_comp, 1}, 8, comp_key);
    }
    int nel_ccomp = wanda_input_file.get_maxdim_index("C_COMPONENTS");
    std::vector<std::string> ccomp(nel_ccomp);
    if (nel_ccomp != 0)
    {
        wanda_input_file.get_string_element("C_COMPONENTS", "C_comp_key", {1, nel_ccomp, 1}, 8, ccomp);
    }
    int nel_sig = wanda_input_file.get_maxdim_index("SIGNAL_LINES");
    std::vector<std::string> sig_lines(nel_sig);
    if (nel_sig != 0)
    {
        wanda_input_file.get_string_element("SIGNAL_LINES", "Sig_line_key", {1, nel_sig, 1}, 8, sig_lines);
    }

    for (int del_node : deleted_phys_nodes)
    {
        if (del_node != -99)
        {
            int index = get_key_index_array(nodes, "N" + int2string(del_node, 7)) + 1;
            if (index != 0)
            {
                wanda_input_file.write_string_elements("H_NODES", "H_node_key", {index, index, 1}, 8, free);
                remove_wdx = true;
            }
        }
    }
    deleted_phys_nodes.clear();
    wanda_input_file.set_int_attribute("H_NODES", "N_group", number_physical_nodes);

    for (int del_comp : deleted_phys_components)
    {
        int index = get_key_index_array(comp_key, "H" + int2string(del_comp, 7)) + 1;
        wanda_input_file.write_string_elements("H_COMPONENTS", "H_comp_key", {index, index, 1}, 8, free);
        remove_wdx = true;
    }
    deleted_phys_components.clear();
    wanda_input_file.set_int_attribute("H_COMPONENTS", "N_group", number_physical_components);
    wanda_input_file.set_int_attribute("H_COM_SPEC_VAL", "N_group", number_physical_components);
    wanda_input_file.set_int_attribute("H_OPE_SPEC_VAL", "N_group", number_physical_components);

    for (int del_comp : deleted_ctrl_components)
    {
        int index = get_key_index_array(ccomp, "C" + int2string(del_comp, 7)) + 1;
        wanda_input_file.write_string_elements("C_COMPONENTS", "C_comp_key", {index, index, 1}, 8, free);
        remove_wdx = true;
    }
    deleted_ctrl_components.clear();
    wanda_input_file.set_int_attribute("C_COMPONENTS", "N_group", number_control_components);

    for (int del_sig : deleted_signal_lines)
    {
        int index = get_key_index_array(sig_lines, "S" + int2string(del_sig, 7)) + 1;
        if (index != 0)
        {
            wanda_input_file.write_string_elements("SIGNAL_LINES", "Sig_line_key", {index, index, 1}, 8, free);
        }
        remove_wdx = true;
    }
    deleted_signal_lines.clear();

    remove_wdx = save_new_phys_comp() || remove_wdx;
    save_phys_comp_input();

    for (auto &comp : ctrl_components)
    {
        if (comp.second.is_new())
        {
            save_new_ctrl_comp(comp.second);
            remove_wdx = true;
        }
        save_ctrl_comp_input(comp.second);
        comp.second.set_new(false);
    }

    remove_wdx = save_new_node() || remove_wdx;
    for (auto &node : phys_nodes)
    {
        save_node_input(node.second);
    }

    for (auto &sig_line : signal_lines)
    {
        if (sig_line.second.is_new())
        {
            save_new_sig_line(sig_line.second);
            remove_wdx = true;
        }
        save_sig_line_input(sig_line.second);
    }

    save_lines_diagram_information();
    save_diagram_text_boxes();

    std::vector<int> Next_seq_nr(1);

    Next_seq_nr[0] = last_key;
    Next_seq_nr[0]++;
    wanda_input_file.write_int_elements("CASE_INFORMATION", "Next_seq_nr", nefis_file::single_elem_uindex, Next_seq_nr);
    
    this->reset_modified();
    std::string wdx = wanda_input_file.get_filename().substr(0, wanda_input_file.get_filename().size() - 3) + "wdx";
    if (remove_wdx)
    {
        if (FileExists(wdx))
        {
            remove(wdx.c_str());
        }
    }
    std::vector<std::string> version;
#ifdef _WIN32
    std::string dll = wanda_bin + "\\steady.exe";
    std::string version_string = get_file_version(dll);
    version.push_back(version_string);
#else
    // todo fix to get it from somewhere else
    version.push_back("4.6.0");
#endif
    wanda_input_file.write_string_elements("WANDA", "Wanda_version", nefis_file::single_elem_uindex, 0, version);
}

void wanda_model::reload_input()
{
    num_cols_loaded = false;
    tables_loaded = false;
    table_metainfo_cache.clear();
    table_metainfo_cache.clear();
    string_col_loaded = false;
    if (!wanda_input_file.is_open())
        wanda_input_file.open();
    load_table();
    read_general_items();
    read_phys_node_input();
    read_phys_component_input();
    read_ctrl_component_input();
    load_lines_diagram_information();
    load_diagram_text_boxes();
    reset_modified();
}

void wanda_model::read_component_output(wanda_component &component)
{
    if (!component_exists(component))
    {
        throw(component.get_complete_name_spec() + " does not exist in Wanda model");
    }
    for (auto &item : component)
    {
        if (item.second.get_species_number() <= num_of_species)
        {
            read_prop_output(item.second);
        }
    }
}

void wanda_model::read_prop_output(wanda_property &item)
{

    if (item.get_property_type() == wanda_property_types::HIS ||
        item.get_property_type() == wanda_property_types::HCS ||
        item.get_property_type() == wanda_property_types::CIS || item.get_property_type() == wanda_property_types::NIS)
        return;
    if (output_quantity_cache.find(item.get_wdo_postfix()) == output_quantity_cache.end())
    {
        std::string group_name = "OUTP_";
        group_name.append(item.get_wdo_postfix());
        std::string group_name_extr = "EXTR_";
        group_name_extr.append(item.get_wdo_postfix());
        int N_values = wanda_output_file.get_int_attribute(group_name, "N_values");
        if (N_values == 0)
            return;
        // num_timesteps = wanda_output_file.get_int_attribute(group_name,
        // "N_timesteps");
        num_timesteps = wanda_output_file.get_int_attribute("OUTPUT_TIME", "N_timesteps");
        wanda_output_data_struct *buffer = new wanda_output_data_struct;
        buffer->time_series_data.resize(N_values, std::vector<float>(num_timesteps));
        buffer->maximum_value_time.resize(N_values, std::vector<float>(1));
        buffer->minimum_value_time.resize(N_values, std::vector<float>(1));
        buffer->minimum_value.resize(N_values, std::vector<float>(1));
        buffer->maximum_value.resize(N_values, std::vector<float>(1));

        if (item.get_property_type() != wanda_property_types::HOV &&
            item.get_property_type() != wanda_property_types::NOV &&
            item.get_property_type() != wanda_property_types::COV)
        {
            wanda_output_file.get_float_element(group_name, "Value", {1, N_values, 1}, {1, num_timesteps, 1},
                                                buffer->time_series_data);
            wanda_output_file.get_float_element(group_name_extr, "T_Value_max", {1, N_values, 1},
                                                nefis_file::single_elem_uindex, buffer->maximum_value_time);
            wanda_output_file.get_float_element(group_name_extr, "T_Value_min", {1, N_values, 1},
                                                nefis_file::single_elem_uindex, buffer->minimum_value_time);
            wanda_output_file.get_float_element(group_name_extr, "Value_max", {1, N_values, 1},
                                                nefis_file::single_elem_uindex, buffer->maximum_value);
            wanda_output_file.get_float_element(group_name_extr, "Value_min", {1, N_values, 1},
                                                nefis_file::single_elem_uindex, buffer->minimum_value);
        }
        else
        {
            wanda_output_file.get_float_element(group_name, "Value", {1, N_values, 1}, nefis_file::single_elem_uindex,
                                                buffer->time_series_data, false);
        }
        output_quantity_cache.emplace(std::make_pair(item.get_wdo_postfix(), *buffer));
        delete buffer;
    }
    auto &outputdata = output_quantity_cache[item.get_wdo_postfix()];
    if (item.get_property_type() == wanda_property_types::HOV ||
        item.get_property_type() == wanda_property_types::COV || item.get_property_type() == wanda_property_types::NOV)
    {
        int index = item.get_group_index() + item.get_hos_index() - 1;
        item.set_scalar_by_ref(outputdata.time_series_data[index][0]);
    }
    else if (item.get_number_of_elements() == 0)
    {
        item.set_series_by_ref(&outputdata.time_series_data[(item.get_group_index() + item.get_hos_index() - 1)]);
        item.set_scalar_by_ref(outputdata.time_series_data[item.get_group_index() + item.get_hos_index() - 1]
                                                          [0]); // Add steady state value as scalar
        item.set_extremes(&outputdata.minimum_value[item.get_group_index() + item.get_hos_index() - 1][0],
                          &outputdata.maximum_value[item.get_group_index() + item.get_hos_index() - 1][0],
                          &outputdata.minimum_value_time[item.get_group_index() + item.get_hos_index() - 1][0],
                          &outputdata.maximum_value_time[item.get_group_index() + item.get_hos_index() - 1][0]);
    }
    else
    {
        for (int i = 0; i <= item.get_number_of_elements(); i++)
        {
            item.set_series_pipe_by_ref(
                i, &outputdata.time_series_data[item.get_group_index() + item.get_hos_index() - 1 + i]);
            item.set_extremes(i, &outputdata.minimum_value[item.get_group_index() + item.get_hos_index() - 1 + i][0],
                              &outputdata.maximum_value[item.get_group_index() + item.get_hos_index() - 1 + i][0],
                              &outputdata.minimum_value_time[item.get_group_index() + item.get_hos_index() - 1 + i][0],
                              &outputdata.maximum_value_time[item.get_group_index() + item.get_hos_index() - 1 + i][0]);
        }
    }
}

void wanda_model::read_node_output(wanda_node &node)
{
    if (!node_exists(node))
    {
        throw(node.get_complete_name_spec() + " does not exist in Wanda model");
    }
    for (auto &item : node)
    {
        read_prop_output(item.second);
    }
}

void wanda_model::reload_output()
{
    if (!FileExists(wanda_output_file.get_filename()))
    {
        throw std::runtime_error("Wanda output file doesn't exist, run steady and/or unsteady "
                                 "computations to create simulation output");
    }
    output_quantity_cache.clear();
    num_timesteps = wanda_output_file.get_int_attribute("OUTPUT_TIME", "N_timesteps");
    simulation_time_steps.resize(num_timesteps);
    wanda_output_file.get_float_element("OUTPUT_TIME", "Value", {1, num_timesteps, 1}, simulation_time_steps);

    for (auto &item : phys_components)
    {
        if (!item.second.is_disused())
            read_component_output(item.second);
    }
    for (auto &item : ctrl_components)
    {
        if (!item.second.is_disused())
            read_component_output(item.second);
    }
    for (auto &item : phys_nodes)
    {
        if (!item.second.is_disused())
            read_node_output(item.second);
    }
}

std::vector<std::string> wanda_model::get_components_name_with_keyword(std::string keyword)
{
    std::vector<std::string> ComponentList;
    for (auto item : phys_components)
    {
        if (item.second.has_keyword(keyword))
        {
            ComponentList.push_back(item.second.get_complete_name_spec());
        }
    }
    return ComponentList;
}

std::vector<wanda_component *> wanda_model::get_components_with_keyword(std::string keyword)
{
    std::vector<wanda_component *> componentList;

    for (auto &item : phys_components)
    {
        if (item.second.has_keyword(keyword))
        {
            componentList.push_back(&item.second);
        }
    }
    for (auto &item : ctrl_components)
    {
        if (item.second.has_keyword(keyword))
        {
            componentList.push_back(&item.second);
        }
    }
    return componentList;
}

std::vector<std::string> wanda_model::get_node_names_with_keyword(std::string keyword)
{
    std::vector<std::string> ComponentList;
    for (auto item : phys_nodes)
    {
        if (item.second.has_keyword(keyword))
        {
            ComponentList.push_back(item.second.get_complete_name_spec());
        }
    }
    return ComponentList;
}

std::vector<wanda_node *> wanda_model::get_nodes_with_keyword(std::string keyword)
{
    std::vector<wanda_node *> ComponentList;
    for (auto &item : phys_nodes)
    {
        if (item.second.has_keyword(keyword))
        {
            ComponentList.push_back(&item.second);
        }
    }
    return ComponentList;
}

std::vector<wanda_sig_line *> wanda_model::get_signal_lines_with_keyword(std::string keyword)
{
    std::vector<wanda_sig_line *> ComponentList;
    for (auto &item : signal_lines)
    {
        if (item.second.has_keyword(keyword))
        {
            ComponentList.push_back(&item.second);
        }
    }
    return ComponentList;
}

std::vector<std::string> wanda_model::get_signal_line_names_with_keyword(std::string keyword)
{
    std::vector<std::string> line_names;
    for (auto &item : signal_lines)
    {
        if (item.second.has_keyword(keyword))
        {
            line_names.push_back(item.second.get_complete_name_spec());
        }
    }
    return line_names;
}
int wanda_model::get_num_time_steps()
{
    float sim_time = get_property("Simulation time").get_scalar_float();
    float start_time = get_property("Start time").get_scalar_float();
    float time_step = get_property("Time step").get_scalar_float();
    float output_inc = get_property("Output increment").get_scalar_float();
    return int(floor(((sim_time - start_time) / time_step) / output_inc) + 1);
}

std::vector<wanda_component *> wanda_model::get_route(std::string keyword, std::vector<int> &dir)
{
    std::vector<wanda_component *> components = get_components_with_keyword(keyword);
    if (components.empty())
        throw std::invalid_argument("No components with keyword " + keyword);
    std::unordered_map<std::string, std::vector<std::string>> nodes;
    std::unordered_map<std::string, std::vector<std::string>> comps;
    int number_odd = 0;
    for (auto comp : components)
    {
        for (int i = 1; i <= comp->get_number_of_connnect_points(); i++)
        {
            auto node = comp->get_connected_node(i);
            comps[comp->get_complete_name_spec()].push_back(node.get_complete_name_spec());
            // if the component is a supplier it is a start point
            if (comp->get_physcomp_type() == "SUPPLIER")
            {
                number_odd++;
            }
            if (nodes.find(node.get_complete_name_spec()) == nodes.end())
            {
                if (comp->get_physcomp_type() != "TEE" && comp->get_physcomp_type() != "CROSS")
                {
                    nodes[node.get_complete_name_spec()].push_back(comp->get_complete_name_spec());
                    number_odd++;
                }
                else
                {
                    for (auto con_comp : node.get_connected_components())
                    {
                        if (con_comp->get_complete_name_spec() != comp->get_complete_name_spec())
                        {
                            if (con_comp->has_keyword(keyword))
                            {
                                nodes[node.get_complete_name_spec()].push_back(comp->get_complete_name_spec());
                                number_odd++;
                            }
                        }
                    }
                }
            }
            else
            {
                nodes[node.get_complete_name_spec()].push_back(comp->get_complete_name_spec());
                if ((nodes[node.get_complete_name_spec()].size() & 1) == 1)
                {
                    number_odd++;
                }
                else
                {
                    number_odd--;
                }
            }
        }
    }
    // this is based on Euler graph Theorem.
    // A finite undirected connected graph is an Euler graph
    // if and only if exactly two vertices are of odd degree or
    // all vertices are of even degree. In the latter case,
    // every Euler path of the graph is a circuit, and in the former case, none
    // is.
    if (number_odd != 0 && number_odd != 2)
    {
        throw std::runtime_error("No route possible with keyword " + keyword);
    }

    std::vector<wanda_component *> comps_ordered;
    std::vector<std::string> comps_ordered_name;
    std::vector<std::string> nodes_ordered_name;
    // std::vector<int> dir;

    if (number_odd == 0)
    {
        // startpoint does not matter
        comps_ordered.push_back(components.at(0));
        nodes_ordered_name.push_back(comps[components.at(0)->get_complete_name_spec()][0]);
        dir.push_back(1);
    }
    else
    {
        for (auto node : nodes)
        {
            if (node.second.size() == 1)
            {
                comps_ordered.push_back(&get_component(node.second[0]));
                nodes_ordered_name.push_back(node.first);
                // check to which connection point the node is connected. The node is the node which is 'outside' the
                // route
                if (comps.at(node.second[0])[0] == node.first)
                {
                    dir.push_back(1);
                }
                else
                {
                    dir.push_back(-1);
                }
                break;
            }
        }
        // no node found only connected to one component, there are Suppliers which shoudl be used as start point
        if (comps_ordered.empty())
        {
            for (auto comp_name : comps)
            {
                auto &comp = get_component(comp_name.first);
                if (comp.get_physcomp_type() == "SUPPLIER")
                {
                    comps_ordered.push_back(&comp);
                    nodes_ordered_name.push_back(comp_name.second[0]);
                    dir.push_back(1);
                    break;
                }
            }
        }
    }
    comps_ordered_name.push_back(comps_ordered.back()->get_complete_name_spec());

    for (int i = 1; i <= comps.size(); i++)
    {
        for (auto node : comps[comps_ordered_name.back()])
        {
            if (std::find(nodes_ordered_name.begin(), nodes_ordered_name.end(), node) == nodes_ordered_name.end())
            {
                nodes_ordered_name.push_back(node);
                break;
            }
        }
        for (auto comp : nodes[nodes_ordered_name.back()])
        {
            if (std::find(comps_ordered_name.begin(), comps_ordered_name.end(), comp) == comps_ordered_name.end())
            {
                comps_ordered.push_back(&get_component(comp));
                comps_ordered_name.push_back(comp);
                int con_point = comps_ordered.back()->get_connect_point(get_node(nodes_ordered_name.back()));
                if (comps_ordered.back()->get_physcomp_type() != "TEE" &&
                    comps_ordered.back()->get_physcomp_type() != "CROSS")
                {
                    dir.push_back(con_point == 1 ? 1 : -1);
                }
                else
                {
                    dir.push_back(0);
                }
                break;
            }
        }
    }
    return comps_ordered;
}

std::vector<wanda_component *> wanda_model::get_all_pipes()
{
    std::vector<wanda_component *> pipes;

    for (auto &comp : phys_components)
    {
        if (comp.second.is_pipe())
        {
            pipes.push_back(&comp.second);
        }
    }
    return pipes;
}

std::vector<wanda_component *> wanda_model::get_all_components()
{
    std::vector<wanda_component *> list;
    for (auto &item : phys_components)
    {
        list.push_back(&item.second);
    }
    for (auto &item : ctrl_components)
    {
        list.push_back(&item.second);
    }
    return list;
}

std::vector<std::string> wanda_model::get_all_components_str()
{
    std::vector<wanda_component *> list = get_all_components();
    std::vector<std::string> strlist;
    for (const auto &item : list)
    {
        strlist.push_back(item->get_complete_name_spec());
    }
    return strlist;
}

std::vector<wanda_node *> wanda_model::get_all_nodes()
{
    std::vector<wanda_node *> list;
    for (auto &item : phys_nodes)
    {
        list.push_back(&item.second);
    }
    return list;
}

std::vector<wanda_sig_line *> wanda_model::get_all_signal_lines()
{
    std::vector<wanda_sig_line *> list;
    for (auto &item : signal_lines)
    {
        list.push_back(&item.second);
    }
    return list;
}

std::vector<std::string> wanda_model::get_all_nodes_str()
{
    std::vector<wanda_node *> list = get_all_nodes();
    std::vector<std::string> strlist;
    for (const auto &item : list)
    {
        strlist.push_back(item->get_complete_name_spec());
    }
    return strlist;
}

std::vector<std::string> wanda_model::get_all_signal_lines_str()
{
    auto list = get_all_signal_lines();
    std::vector<std::string> strlist;
    for (const auto &item : list)
    {
        strlist.push_back(item->get_complete_name_spec());
    }
    return strlist;
}

void wanda_model::new_wanda_case(std::string casename)
{
    if (wanda_input_file.is_open())
    {
        wanda_input_file.close();
    }
    if (wanda_output_file.is_open())
    {
        wanda_output_file.close();
    }
    std::array<std::string, 9> extensions = {
        "wdi", "wdo", "wdx", "wdd", "wmf", "_um", "_sm", "__I", "__R",
    };
    for (auto &ext : extensions)
    {
        std::string filename = casename.substr(0, casename.size() - 3) + ext;
        remove(filename.c_str());
    }
    // copy untitled wdi and rename
    const std::filesystem::path from(component_definition->get_data_path() + "untitled.wdi");
    const std::filesystem::path to(casename);

    // Check both paths in order to give a clear error message to the user. the
    // eventual error thrown by the try_catch on std::copy_file only says "invalid
    // arguments"
    if (!FileExists(from.generic_string()))
        throw std::invalid_argument("Untitled.wdi doesn't exist in Wanda bin directory: " + from.generic_string());
    if (!FileExists(to.parent_path().generic_string()))
        throw std::invalid_argument("Directory of case file doesn't exist: " + to.generic_string());

    try
    {
        copy_file(from, to, std::filesystem::copy_options::overwrite_existing);
    }
    catch (std::exception &e)
    {
        throw e.what();
    }
    num_of_species = 0;
    number_physical_components = 0;
    number_control_components = 0;
    number_physical_nodes = 0;
    num_signal_lines = 0;
    phys_nodes.clear();
    ctrl_components.clear();
    phys_components.clear();
    name2_phys_comp.clear();
    name2_phys_node.clear();
    signal_lines.clear();
    tables_loaded = false;
    table_metainfo_cache.clear();
    num_cols_loaded = false;
    table_metainfo_cache.clear();
    string_col_loaded = false;
    table_metainfo_cache.clear();
    global_vars.clear();
    mode_and_opt.clear();
    std::unordered_map<std::string, std::string> unit_list = component_definition->get_case_unit("UNIT_GROUP_USER");
    std::vector<std::string> unit_descr;
    std::vector<std::string> unit_dims;

    for (auto unit : unit_list)
    {
        unit_dims.push_back(unit.first);
        unit_descr.push_back(unit.second);
    }
    wanda_input_file.set_file(casename);
    //    wanda_input_file.open();
    initialize(casename, false);
    wanda_input_file.write_string_elements("CASE_INFORMATION", "User_unit_descr", nefis_file::single_elem_uindex, 0,
                                           unit_descr);
    wanda_input_file.write_string_elements("CASE_INFORMATION", "User_unit_dims", nefis_file::single_elem_uindex, 0,
                                           unit_dims);

    mode_and_options_base_data mode_option_list;
    mode_and_opt = mode_option_list.get_default_list();
    get_globvar_data();
    last_key = 0;
    new_case_statusflag = true;
}

wanda_component &wanda_model::add_component(const std::string type_name, const std::vector<float> position)
{
    std::string Class_sort_key = component_definition->get_class_sort_key(type_name);
    if (component_definition->is_obsolete(Class_sort_key))
    {
        throw std::invalid_argument(type_name + " is obsolete component");
    }
    if (component_definition->is_physical_component(Class_sort_key))
    {
        std::string compkey = get_unique_key(&phys_components, 'H', last_key);
        auto key = strtol(compkey.substr(1).c_str(), nullptr, 10);
        auto Name_prefix = component_definition->get_name_prefix_phys_comp(Class_sort_key);
        std::string class_name = component_definition->get_class_name_phys_comp(Class_sort_key);
        std::string type = component_definition->get_type_phys_comp(Class_sort_key);

        int j = last_key;
        std::string Name = Name_prefix.substr(0, 1) + std::to_string(last_key);
        while (check_name(Name_prefix + " " + Name_prefix + " " + Name, compkey))
        {
            j++;
            Name = Name_prefix.substr(0, 1) + std::to_string(j);
        }

        // wanda_component comp(key, class_name, Class_sort_key, Name_prefix, Name,
        //                      physical, type, type_name);
        // phys_components.emplace(
        //     compkey, comp);  // wanda_component(key, class_name, Class_sort_key,
        // Name_prefix, Name, physical, type, type_name));
        wanda_component *comp_point = component_definition->get_component(Class_sort_key);
        phys_components.emplace(compkey, *comp_point);
        phys_components[compkey].set_comp_key(key);
        phys_components[compkey].set_name(Name);
        phys_components[compkey].set_new(true);
        phys_components[compkey].set_modified(true);
        phys_components[compkey].set_position(position);
        if (type_name == "Multi species supplier tank")
        {
            num_of_species += 1;
            if (num_of_species > max_num_of_species)
            {
                throw std::runtime_error("Number of different species is greater than the maximum which is " +
                                         std::to_string(max_num_of_species));
            }
        }

        number_physical_components++;

        for (auto &input : phys_components[compkey])
        {
            input.second.set_unit_factor(unit_list, case_units);
            if (input.second.get_property_type() != wanda_property_types::HIS)
                continue;

            input.second.set_modified(true);
            if (input.second.has_table())
            {
                input.second.get_table().set_modified(true);
            }
        }
        // comp_name_2_comp.emplace(comp.get_complete_name_spec(),
        // &phys_components[compkey]);
        name2_phys_comp.emplace(phys_components[compkey].get_complete_name_spec(), &phys_components[compkey]);
        return phys_components[compkey];
    }
    if (component_definition->is_control_component(Class_sort_key))
    {
        // switching the modle to control mode when a control component is added.
        if (get_property("Control").get_scalar_float() == 0.0)
        {
            get_property("Engineering mode").set_scalar(0.0);
            get_property("Transient mode").set_scalar(1.0);
            get_property("Control").set_scalar(1.0);
            get_property("Cavitation").set_scalar(1.0);
        }

        std::string compkey = get_unique_key(&ctrl_components, 'C', last_key);
        auto key = strtol(compkey.substr(1).c_str(), nullptr, 10);
        auto Name_prefix = component_definition->get_name_prefix_ctrl_comp(Class_sort_key);
        std::string class_name = component_definition->get_class_name_ctrl_comp(Class_sort_key);
        std::string type = type_name;

        int j = last_key;
        std::string Name = Name_prefix.substr(0, 1) + std::to_string(last_key);
        while (check_name(Name_prefix + " " + Name, compkey))
        {
            j++;
            Name = Name_prefix.substr(0, 1) + std::to_string(j);
        }
        wanda_component comp(key, class_name, Class_sort_key, Name_prefix, Name, wanda_type::control, type, type_name,
                             component_definition);
        comp.set_new(true);
        comp.set_position(position);
        comp.set_max_input_channels(component_definition->get_max_in_chan(Class_sort_key));
        comp.set_input_channel_type(component_definition->get_in_chan_type(Class_sort_key));
        comp.set_output_channel_type(component_definition->get_out_chan_type(Class_sort_key));
        ctrl_components.emplace(compkey, comp);
        number_control_components++;

        for (auto &input : ctrl_components[compkey])
        {
            input.second.set_unit_factor(unit_list, case_units);
            if (input.second.get_property_type() != wanda_property_types::CIS)
                continue;
            if (input.second.get_property_spec_inp_fld() == 'C')
            {
                input.second.set_scalar(float(1));
            }
            input.second.set_modified(true);
        }
        // comp_name_2_comp.emplace(comp.get_complete_name_spec(),
        // &ctrl_components[compkey]);
        return ctrl_components[compkey];
    }
    throw std::invalid_argument(type_name + " is not a Wanda component");
}

wanda_component &wanda_model::add_component(std::string type, std::vector<float> position, std::string name)
{
    auto &comp = add_component(type, position);

    if (check_name(comp.get_name_prefix() + " " + name, comp.get_key_as_string()))
    {
        if (comp.get_item_type() == wanda_type::physical)
        {
            phys_components.erase(comp.get_key_as_string());
            name2_phys_comp.erase(comp.get_complete_name_spec());
        }
        else
        {
            ctrl_components.erase(comp.get_key_as_string());
        }

        throw std::invalid_argument("Component with name " + name + " already exists");
    }

    if (comp.get_item_type() == wanda_type::physical)
    {
        name2_phys_comp.erase(comp.get_complete_name_spec());
        comp.set_name(name);
        name2_phys_comp.emplace(comp.get_complete_name_spec(), &phys_components[comp.get_key_as_string()]);
        return phys_components[comp.get_key_as_string()];
    }
    comp.set_name(name);
    return ctrl_components[comp.get_key_as_string()];
}

wanda_component &wanda_model::add_component(wanda_component *comp_org, std::vector<float> position)
{
    auto new_comp_temp = add_component(comp_org->get_type_name(), position);
    // check if comp_key has been used if not adjust the comp key of the new component.
    wanda_component *new_comp = nullptr;
    if (comp_org->get_item_type() == wanda_type::physical)
    {
        if (phys_components.find(comp_org->get_key_as_string()) == phys_components.end())
        {
            phys_components.erase(new_comp_temp.get_key_as_string());
            new_comp_temp.set_comp_key(comp_org->get_key());
            phys_components.emplace(new_comp_temp.get_key_as_string(), new_comp_temp);
        }
        new_comp = &phys_components.at(new_comp_temp.get_key_as_string());
    }
    else if ((comp_org->get_item_type() == wanda_type::control))
    {
        if (ctrl_components.find(comp_org->get_key_as_string()) == ctrl_components.end())
        {
            ctrl_components.erase(new_comp_temp.get_key_as_string());
            new_comp_temp.set_comp_key(comp_org->get_key());
            ctrl_components.emplace(new_comp_temp.get_key_as_string(), new_comp_temp);
        }
        new_comp = &ctrl_components.at(new_comp_temp.get_key_as_string());
    }
    else
    {
        throw std::runtime_error("Unknown component type");
    }

    std::string compkey = new_comp->get_key_as_string();
    int i = 0;
    std::string name = comp_org->get_complete_name_spec();
    if (!(name == new_comp->get_complete_name_spec()))
    {
        while (check_name(name, compkey))
        {
            // name already used, comp is renamed
            if (name.size() > 128 - 3)
            {
                name = comp_org->get_name().substr(0, 128 - 3) + "_" + std::to_string(i);
            }
            else
            {
                name = comp_org->get_name() + "_" + std::to_string(i);
            }
            i++;
        }
    }
    if (i == 0)
    {
        name = comp_org->get_name();
    }
    new_comp->copy_input(*comp_org);
    // copy keywords
    for (auto keyword : comp_org->get_keywords())
    {
        new_comp->add_keyword(keyword);
    }
    if (comp_org->get_item_type() == wanda_type::physical)
    {
        name2_phys_comp.erase(new_comp->get_complete_name_spec());
        new_comp->set_name(name);
        name2_phys_comp.emplace(new_comp->get_complete_name_spec(), &phys_components[compkey]);
        phys_components[compkey].set_number_of_species(&num_of_species);
        return phys_components[compkey];
    }
    if (comp_org->get_item_type() == wanda_type::control)
    {
        new_comp->set_name(name);
        return ctrl_components[compkey];
    }
    throw std::runtime_error(comp_org->get_complete_name_spec() + " is not a control or physical component");
}

wanda_node &wanda_model::add_node(std::string type, std::vector<float> position)
{
    std::string Class_sort_key = component_definition->get_class_sort_key(type);
    if (component_definition->is_obsolete(Class_sort_key))
    {
        throw std::invalid_argument(type + " is obsolete component");
    }
    std::string compkey = get_unique_key(&phys_nodes, 'N', last_key);
    auto key = strtol(compkey.substr(1).c_str(), nullptr, 10);
    auto Name_prefix = component_definition->get_name_prefix_phys_node(Class_sort_key);
    std::string class_name = component_definition->get_class_name_phys_node(Class_sort_key);

    int j = last_key; // last key is used to generate new unique keys for comps,
                      // nodes siglines tables. This value is also saved in the wdi.
    std::string Name = Name_prefix.substr(0, 1) + std::to_string(j);
    while (check_name(Name_prefix + " " + Name, compkey))
        Name = Name_prefix.substr(0, 1) + std::to_string(++j);

    // wanda_node new_node(key, class_name, Class_sort_key, Name_prefix, Name, type);
    wanda_node *new_node = component_definition->get_node(Class_sort_key);
    phys_nodes.try_emplace(compkey, *new_node);
    phys_nodes[compkey].set_comp_key(key);
    phys_nodes[compkey].set_name(Name);
    phys_nodes[compkey].set_new(true);
    phys_nodes[compkey].set_modified(true);
    phys_nodes[compkey].set_position(position);

    for (auto &input : phys_nodes[compkey].get_all_properties())
    {
        if (input->get_property_type() != wanda_property_types::NIS)
            continue;
        if (input->get_property_spec_inp_fld() == 'C')
            input->set_scalar(float(1));
        input->set_modified(true);
    }

    // node_name_2_node.try_emplace(new_node.get_complete_name_spec(),
    // &phys_nodes[compkey]);
    name2_phys_node.emplace(phys_nodes[compkey].get_complete_name_spec(), &phys_nodes[compkey]);
    number_physical_nodes++;
    return phys_nodes[compkey];
}

wanda_node &wanda_model::add_node(wanda_node *node_org, std::vector<float> position)
{
    auto new_node_temp = add_node(node_org->get_type_name(), position);
    // chaning the key to the key of the orginal node if it does not exist already
    if (phys_nodes.find(node_org->get_key_as_string()) == phys_nodes.end())
    {
        phys_nodes.erase(new_node_temp.get_key_as_string());
        new_node_temp.set_comp_key(node_org->get_key());
        phys_nodes.emplace(new_node_temp.get_key_as_string(), new_node_temp);
    }
    auto &new_node = phys_nodes.at(new_node_temp.get_key_as_string());
    for (auto keyword : node_org->get_keywords())
    {
        new_node.add_keyword(keyword);
    }
    std::string compkey = new_node.get_key_as_string();
    int i = 0;
    std::string name = node_org->get_complete_name_spec();
    if (!(name == new_node.get_complete_name_spec()))
    {
        while (check_name(name, compkey))
        {
            // name already used, comp is renamed
            if (name.size() > 128 - 3)
            {
                name = node_org->get_name().substr(0, 128 - 3) + "_" + std::to_string(i);
            }
            else
            {
                name = node_org->get_name() + "_" + std::to_string(i);
            }
            i++;
        }
    }
    if (i == 0)
    {
        name = node_org->get_name();
    }
    name2_phys_node.erase(new_node.get_complete_name_spec());
    new_node.set_name(name);
    name2_phys_node.emplace(name, &new_node);
    new_node.copy_input(*node_org);
    return new_node;
}

 wanda_diagram_lines &wanda_model::add_diagram_line(wanda_item *item, std::string from_key, std::string to_key,
                                                      std::vector<float> x, std::vector<float> y, int color,
                                                      int line_thickness)
{
     auto key = get_unique_key(&diagram_lines, 'L', last_key);
    diagram_lines.emplace(key, wanda_diagram_lines(key, item, from_key, to_key,  x.size(), x, y, color, line_thickness));
    return diagram_lines[key];
 }

diagram_text& wanda_model::add_text_box(text_field text, int bck_color, int line_thickness, coordinates coor, float width,
                          float height)
{
    auto key = get_unique_key(&diagram_lines, 'T', last_key);
    diagram_text_boxes.insert(std::pair(key, diagram_text(key, text, bck_color, line_thickness, coor, width, height)));
    return diagram_text_boxes[key];
}

void wanda_model::delete_component(wanda_component &component)
{
    if (!component_exists(component))
    {
        throw(component.get_complete_name_spec() + " does not exist in Wanda model");
    }
    if (phys_components.find(component.get_key_as_string()) != phys_components.end())
    {
        for (int i = 1; i <= component.get_number_of_connnect_points(); i++)
        {
            disconnect(component, i);
        }
        if (component.get_type_name() == "Multi species supplier tank")
        {
            num_of_species -= 1;
        }
        deleted_phys_components.push_back(component.get_key());
        name2_phys_comp.erase(component.get_complete_name_spec());
        phys_components.erase(component.get_key_as_string());
        number_physical_components -= 1;
    }
    else if (ctrl_components.find(component.get_key_as_string()) != ctrl_components.end())
    {
        for (int i = 1; i <= component.get_num_input_channels(); i++)
        {
            disconnect(component, i, true);
        }
        for (int i = 1; i <= component.get_num_output_channels(); i++)
        {
            disconnect(component, i, false);
        }
        deleted_ctrl_components.push_back(component.get_key());
        ctrl_components.erase(component.get_key_as_string());
        number_control_components += -1;
    }
}

void wanda_model::delete_node(wanda_node &node)
{
    if (!node_exists(node))
    {
        throw(node.get_complete_name_spec() + " does not exist in Wanda model");
    }
    auto list = node.get_connected_components();
    for (auto &comp : list)
    {
        comp->disconnect(node);
    }
    deleted_phys_nodes.push_back(node.get_key());
    name2_phys_node.erase(node.get_complete_name_spec());
    phys_nodes.erase(node.get_key_as_string());
}

void wanda_model::delete_sig_line(wanda_sig_line &sig_line)
{
    if (!sig_line_exists(sig_line))
    {
        throw(sig_line.get_complete_name_spec() + " does not exist in Wanda model");
    }
    auto input_con_point = sig_line.get_input_connection_point();
    auto output_con_point = sig_line.get_output_connection_point();
    if (sig_line.get_input_component() != nullptr)
    {
        sig_line.get_input_component()->disconnect(input_con_point, true);
    }
    if (sig_line.get_output_component() != nullptr)
    {
        sig_line.get_output_component()->disconnect(output_con_point, false);
    }

    deleted_signal_lines.push_back(sig_line.get_key());
    auto loc = std::find(sig_line_keys.begin(), sig_line_keys.end(), sig_line.get_key_as_string());
    sig_line_keys.erase(loc);
    signal_lines.erase(sig_line.get_key_as_string());
}

wanda_item &wanda_model::connect(wanda_component &component1, int connection_point1, wanda_component &component2,
                                 int connection_point2)
{
    if (!component_exists(component1))
    {
        throw(component1.get_complete_name_spec() + " does not exist in Wanda model");
    }
    if (!component_exists(component2))
    {
        throw(component2.get_complete_name_spec() + " does not exist in Wanda model");
    }
    if (component1.get_key_as_string() == component2.get_key_as_string())
    {
        throw std::invalid_argument("Cannot attach component to itself");
    }

    if (component1.get_item_type() == wanda_type::physical &&
        component2.get_item_type() == wanda_type::physical) // Two physical components
    {
        return connect_phys_comps(component1, connection_point1, component2, connection_point2);
    }
    if (component1.get_item_type() == wanda_type::control &&
        component2.get_item_type() == wanda_type::control) // Two control components
    {
        return connect_ctrl_comps(component1, connection_point1, component2, connection_point2);
    }
    if (component1.get_item_type() == wanda_type::physical && component2.get_item_type() == wanda_type::control)
    {
        // physical to control can only be sensor
        if (component2.get_class_sort_key().compare("SENSOR") == 0) // component1 is a sensor
        {
            component2.fill_sensor_list(component1, connection_point1);
            return connect_sensor(component1, connection_point1, component2, connection_point2);
        }
        throw std::invalid_argument("Invalid components given, cannot connect");
        // connecting physical component to control is not allowed, must be done via
        // sensor.
    }
    // last case: component1.get_item_type() == wanda_type::control && component2.get_item_type() ==
    // wanda_type::physical
    return connect_ctrl2phys_comps(component1, connection_point1, component2, connection_point2);
}

wanda_item &wanda_model::connect(wanda_component &component1, int connection_point1, wanda_component &component2,
                                 int connection_point2, std::string node_name)
{
    if (node_exists(node_name))
    {
        throw std::invalid_argument(node_name + " already exists in the model");
    }
    auto &item = connect(component1, connection_point1, component2, connection_point2);

    // need to check if it is a node or a signal line
    if (node_exists(item.get_complete_name_spec()))
    {
        change_node_name(get_node(item.get_complete_name_spec()), node_name);
    }
    // TODO change signal line name.
    return item;
}

void wanda_model::connect(wanda_component &component1, const int connection_point1, wanda_node &node)
{
    // todo check if connecting sensor to node!.
    if (phys_components.find(component1.get_key_as_string()) == phys_components.end()) // only has to check
    {
        if (ctrl_components.find(component1.get_key_as_string()) != ctrl_components.end())
        {
            if (component1.get_class_sort_key() == "SENSOR") // component1 is a sensor
            {
                component1.fill_sensor_list(node);
                connect_sensor(node, component1);
                return;
            }
        }
        throw std::invalid_argument("Component " + component1.get_complete_name_spec() +
                                    " is not a physical component in this wanda model");
    }
    if (phys_nodes.find(node.get_key_as_string()) == phys_nodes.end())
        throw std::invalid_argument("Node " + node.get_complete_name_spec() + " is not found in this wanda model");
    if (component1.get_number_of_connnect_points() < connection_point1)
        throw std::invalid_argument(component1.get_complete_name_spec() + " does not have connection point " +
                                    std::to_string(connection_point1));
    std::string core_q1 = component1.get_core_quants(connection_point1);
    std::string node_core_quants = node.get_node_type();

    if ((core_q1).compare((node_core_quants)) != 0)
        throw std::invalid_argument("Connect points are not of the same type of " +
                                    component1.get_complete_name_spec() + " and " + node.get_complete_name_spec());
    for (int i = 1; i <= component1.get_number_of_connnect_points(); i++)
    {
        if (component1.is_node_connected(i))
        {
            if (component1.get_connected_node(i).get_complete_name_spec() == node.get_complete_name_spec())
            {
                throw std::invalid_argument("Node " + node.get_complete_name_spec() +
                                            " is already connected to component " +
                                            component1.get_complete_name_spec());
            }
        }
    }
    if (component1.is_node_connected(connection_point1)) // already connected, remove connected node.
    {
        wanda_node &connected_node = static_cast<wanda_node &>(component1.get_connected_node(connection_point1));
        connected_node.disconnect(component1);
        component1.disconnect(connection_point1);
    }
    component1.connect(node, connection_point1);
    node.connect(component1);
}

void wanda_model::merge_nodes(wanda_node &node2, wanda_node &node1)
{

    if (!node_exists(node1))
    {
        throw(node1.get_complete_name_spec() + " does not exist in Wanda model");
    }
    if (!node_exists(node2))
    {
        throw(node2.get_complete_name_spec() + " does not exist in Wanda model");
    }
    for (auto comp : node1.get_connected_components())
    {
        int con_point = comp->get_connect_point(node1);
        comp->disconnect(con_point);
        node1.disconnect(*comp);
        connect(*comp, con_point, node2);
    }
    delete_node(node1);
}

void wanda_model::disconnect(wanda_component &component, int connection_point)
{
    if (phys_components.find(component.get_key_as_string()) == phys_components.end())
        throw std::invalid_argument("Component " + component.get_complete_name_spec() +
                                    " is not found in this wanda model");

    if (component.get_item_type() == wanda_type::physical)
    {
        if (connection_point <= component.get_number_of_connnect_points())
        {
            if (component.is_node_connected(connection_point))
            {
                auto &connected_node = static_cast<wanda_node &>(component.get_connected_node(connection_point));
                connected_node.disconnect(component);
                component.disconnect(connection_point);
            }
        }
        else
        {
            std::vector<wanda_sig_line *> connect_sig_lines = component.get_connected_sigline(connection_point, true);
            for (wanda_sig_line *sigline : connect_sig_lines)
            {
                delete_sig_line(*sigline);
            }
        }
    }
    else
    {
        throw std::invalid_argument("Component " + component.get_complete_name_spec() + " is not a physical component");
    }
}

void wanda_model::disconnect(wanda_component &component, int connection_point, bool input)
{
    if (ctrl_components.find(component.get_key_as_string()) == ctrl_components.end())
        throw std::invalid_argument("Component " + component.get_complete_name_spec() +
                                    " is not found a ctrl component in this wanda model");
    if (component.is_sigline_connected(connection_point, input))
    {
        auto connect_sig_lines = component.get_connected_sigline(connection_point, input);
        for (auto &line : connect_sig_lines)
        {
            delete_sig_line(*line);
        }
    }
}

wanda_node &wanda_model::connect_phys_comps(wanda_component &comp1, int con_point1, wanda_component &comp2,
                                            int con_point2)
{

    if (comp1.get_number_of_connnect_points() < con_point1)
        throw std::invalid_argument(comp1.get_complete_name_spec() + " does not have connection point " +
                                    std::to_string(con_point1));
    if (comp2.get_number_of_connnect_points() < con_point2)
        throw std::invalid_argument(comp2.get_complete_name_spec() + " does not have connection point " +
                                    std::to_string(con_point2));
    std::string core_q1 = comp1.get_core_quants(con_point1);
    std::string core_q2 = comp2.get_core_quants(con_point2);
    if (core_q1.compare(core_q2) != 0)
        throw std::invalid_argument("Connect points are not of the same type of " + comp1.get_complete_name_spec() +
                                    " and " + comp2.get_complete_name_spec());

    if (!comp1.is_node_connected(con_point1) && !comp2.is_node_connected(con_point2))
    { // both components do not have a connection a new node
      // must be created
        std::string node_typename;
        for (std::string node : standard_nodes)
        {
            std::string core_quants =
                component_definition->get_node_type(component_definition->get_class_sort_key(node));
            if (core_quants.compare(core_q1) == 0)
            {
                node_typename = node;
                break;
            }
        }
        auto position1 = comp1.get_position();
        auto position2 = comp2.get_position();
        float newx = (position1[0] + position2[0]) / 2; // setting the node betwen the two compoennts it connects
                                                        // to so that it looks short of nice in the model
        float newy = (position1[1] + position2[1]) / 2;
        std::vector<float> new_position = {newx, newy};
        wanda_node &new_node = add_node(node_typename, new_position);

        comp1.connect(new_node, con_point1);
        comp2.connect(new_node, con_point2);
        new_node.connect(comp1);
        new_node.connect(comp2);
        return new_node;
    }
    if (comp1.is_node_connected(con_point1) &&
        !comp2.is_node_connected(con_point2)) // comp 1 has already a connection on the connect point,
                                              // no new nodes needs to be created
    {
        wanda_node &node = static_cast<wanda_node &>(comp1.get_connected_node(con_point1));
        comp2.connect(node, con_point2);
        node.connect(comp2);
        comp2.set_modified(true);
        return node;
    }
    if (!comp1.is_node_connected(con_point1) &&
        comp2.is_node_connected(con_point2)) // component2 has already a connection
                                             // no new node has to be created
    {
        wanda_node &node = static_cast<wanda_node &>(comp2.get_connected_node(con_point2));
        comp1.connect(node, con_point1);
        node.connect(comp1);
        comp1.set_modified(true);
        return node;
    }
    if (comp1.is_node_connected(con_point1) &&
        comp2.is_node_connected(con_point2)) // both component have already a connection
    {
        if (comp1.get_connected_node(con_point1).get_key() != comp2.get_connected_node(con_point2).get_key())
        {
            throw std::invalid_argument("Both components are already connected to different nodes.");
        }
        return static_cast<wanda_node &>(comp1.get_connected_node(con_point1));
    }
    throw std::runtime_error("Error occcured during connecting two components");
}

wanda_sig_line &wanda_model::connect_ctrl_comps(wanda_component &comp1, int con_point1, wanda_component &comp2,
                                                int con_point2)
{
    std::string type1 = comp1.get_output_channel_type(con_point1);
    std::string type2 = comp2.get_input_channel_type(con_point2);
    if (type1 != type2)
    {
        throw std::runtime_error("It is not possible to connect to different type of components");
    }
    // check if components are already connected if so no signal line will be created
    for (auto &sig_line : comp1.get_connected_sigline(con_point1, false))
    {
        if (sig_line->get_input_component()->get_complete_name_spec() == comp2.get_complete_name_spec())
        {
            return *sig_line;
        }
    }
    // check if both components can have more signal lines if not throw error
    if (comp2.is_sigline_connected(con_point2, true))
    {
        if (comp2.get_max_input_channels(con_point2) <= comp2.get_connected_sigline(con_point2, true).size())
        {
            throw std::runtime_error("No more input signal lines can be connected to component " +
                                     comp2.get_complete_name_spec() + "connect point " + std::to_string(con_point2));
        }
    }
    // connect components and created new signal line and return it

    auto position1 = comp1.get_position();
    auto position2 = comp2.get_position();
    float newx = (position1[0] + position2[0]) / 2; // setting position of signal line right between the two
                                                    // compoents so that in the UI is short of looks OK.
    float newy = (position1[1] + position2[1]) / 2;
    std::vector<float> new_position = {newx, newy};
    wanda_sig_line &new_sig_line = add_sigline(type1, new_position);

    comp1.connect(new_sig_line, con_point1, false);
    comp2.connect(new_sig_line, con_point2, true);
    new_sig_line.connect_output(comp1, con_point1);
    new_sig_line.connect_input(comp2, con_point2);
    return new_sig_line;
}

wanda_sig_line &wanda_model::connect_ctrl2phys_comps(wanda_component &c_comp1, int con_point1, wanda_component &h_comp2,
                                                     int con_point2)
{
    std::string sigline_typename = c_comp1.get_output_channel_type(con_point1);
    if (sigline_typename.compare(h_comp2.get_ctrl_input_type()) != 0 &&
        h_comp2.get_ctrl_input_type().compare("both") != 0)
        throw std::invalid_argument("Control connectionpoints are not the same type");
    if (con_point2 != h_comp2.get_number_of_connnect_points() + 1)
    {
        throw std::invalid_argument("Control component can only be attached to control connect point of "
                                    "component");
    }
    if (!h_comp2.is_controlable())
    {
        throw std::invalid_argument(h_comp2.get_complete_name_spec() + " is not controlable");
    }

    auto position1 = c_comp1.get_position();
    auto position2 = h_comp2.get_position();
    float newx = (position1[0] + position2[0]) / 2;
    float newy = (position1[1] + position2[1]) / 2;
    std::vector<float> new_position = {newx, newy};
    wanda_sig_line &new_sig_line = add_sigline(sigline_typename, new_position);

    c_comp1.connect(new_sig_line, con_point1, false);
    h_comp2.connect(new_sig_line, con_point2, true);
    new_sig_line.connect_output(c_comp1, con_point1);
    new_sig_line.connect_input(h_comp2, con_point2);

    return new_sig_line; // is this the correct return value???
}

wanda_sig_line &wanda_model::add_sigline(std::string type, std::vector<float> pos)
{
    std::string compkey = get_unique_key(sig_line_keys, 'S', last_key);
    sig_line_keys.push_back(compkey);

    auto key = strtol(compkey.substr(1).c_str(), nullptr, 10);
    int j = last_key; // last key is used to generate new unique comp_keys, table
                      // ekays etc.
    std::string Name = "S" + std::to_string(j);
    while (check_name("Signal " + Name, compkey))
        Name = "S" + std::to_string(++j);

    wanda_sig_line new_sig_line(key, Name);
    new_sig_line.set_signal_line_type(type);
    new_sig_line.set_new(true);
    new_sig_line.set_modified(true);
    new_sig_line.set_position(pos);

    signal_lines.try_emplace(compkey, new_sig_line);
    // sig_line_name_2_sig_line.emplace(new_sig_line.get_complete_name_spec(),
    // &signal_lines[compkey]);
    num_signal_lines++;
    return signal_lines[compkey];
}

void run_external_program_win(std::string exepath, std::string args)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    LPVOID lpMsgBuf;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    std::string commandline(exepath);
    commandline.append(" ").append(args);

    if (exepath.length() > MAX_PATH)
        throw std::runtime_error("Executable path is longer than MAX_PATH: " + exepath);
    if ((commandline.size() + 1) > 32768) // limit accoording to documentation
        throw std::runtime_error("length of path = " + std::to_string(commandline.length()) + " max=32768");
    char *stringbuf = new char[commandline.size() + 1];
    strcpy_s(stringbuf, commandline.size() + 1, commandline.c_str());
    DWORD exitcode;
    if (!CreateProcessA(NULL,             // No module name (use command line)
                        stringbuf,        // Command line
                        NULL,             // Process handle not inheritable
                        NULL,             // Thread handle not inheritable
                        false,            // Set handle inheritance to FALSE
                        CREATE_NO_WINDOW, // Creation flags
                        NULL,             // Use parent's environment block
                        NULL,             // Use parent's starting directory
                        &si,              // Pointer to STARTUPINFO structure
                        &pi)              // Pointer to PROCESS_INFORMATION structure
    )
    {
        exitcode = GetLastError();
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, exitcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL);
        std::string errormsg(static_cast<char *>(lpMsgBuf));
        LocalFree(lpMsgBuf); // because FORMAT_MESSAGE_ALLOCATE_BUFFER flag
        throw std::runtime_error("CreateProcess failed: " + std::to_string(exitcode) + " = " + errormsg +
                                 "path=" + exepath);
    }
    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &exitcode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    delete[] stringbuf;
    if (exitcode != 0)
        throw std::runtime_error("Process exit code: " + std::to_string(exitcode) + '\n');
}

void wanda_model::run_steady()
{
    if (this->is_modified())
    {
        save_model_input();
    }
    wanda_input_file.close();
    if (wanda_output_file.is_open())
    {
        wanda_output_file.close();
    }

    std::string file = wanda_input_file.get_filename();
    auto sl = file.find_last_of('\\');
    if (sl > file.size())
    {
        sl = file.find_last_of('/');
    }

    std::string casename =
        wanda_input_file.get_filename().substr(sl + 1, wanda_input_file.get_filename().size() - 4 - sl - 1);
    std::string exe = "\"" + wanda_bin + "steady.exe" + "\"";
    std::string command_line = " \"" + wanda_input_file.get_filename() + "\" \"";
#ifdef _WINDOWS
    try
    {
        run_external_program_win(exe, command_line);
    }
    catch (std::exception &ex)
    {
        throw(ex);
    }
#else
    try
    {
        command_line = "\"" + exe + command_line + "\"";
        const char *command = command_line.c_str();
        system(command);
    }
    catch (std::exception &ex)
    {
        throw(ex);
    }
#endif
    wanda_input_file.open();
    wanda_output_file.open();
    re_calculate_hcs();
    load_steady_messages();
    std::vector<int> status_steady(1);
    wanda_input_file.get_int_element("STATUS", "Status_steady", nefis_file::single_elem_uindex, status_steady);
    if (status_steady[0] == -1)
    {
        reload_component_indices();
        reload_output();
    }
    else
    {
        throw std::runtime_error("Steady has encountered a terminating error");
    }
}

void wanda_model::run_unsteady()
{
    float trans = get_property("Transient mode").get_scalar_float();
    if (trans != 1.0)
    {
        throw std::runtime_error("Model is not set to transient mode"); // TODO really throw an error?  maybe set a
                                                                        // status and return? send a message to user?
    }
    // check if steady has been run
    std::vector<int> status_unsteady(1);
    wanda_input_file.get_int_element("STATUS", "Status_unsteady", nefis_file::single_elem_uindex, status_unsteady);
    if (this->is_modified() || status_unsteady[0] == 0)
    {
        save_model_input();
        run_steady();
    }

    wanda_input_file.get_int_element("STATUS", "Status_unsteady", nefis_file::single_elem_uindex, status_unsteady);

    if (status_unsteady[0] == 0)
    {
        // check if there is an steady state error
        load_steady_messages();
        for (auto item : phys_components)
        {
            auto messages = item.second.get_all_messages();
            for (auto message : messages)
            {
                if (message.message_type == 'E' || message.message_type == 'T')
                {
                    throw std::runtime_error("Steady error in physical component: " +
                                             item.second.get_complete_name_spec() + message.message);
                }
            }
        }
        for (auto item : ctrl_components)
        {
            auto messages = item.second.get_all_messages();
            for (auto message : messages)
            {
                if (message.message_type == 'E' || message.message_type == 'T')
                {
                    throw std::runtime_error(
                        "Steady error in control component: " + item.second.get_complete_name_spec() + message.message);
                }
            }
        }
        for (auto item : phys_nodes)
        {
            auto messages = item.second.get_all_messages();
            for (auto message : messages)
            {
                if (message.message_type == 'E' || message.message_type == 'T')
                {
                    throw std::runtime_error("Steady error in node: " + item.second.get_complete_name_spec() +
                                             message.message);
                }
            }
        }
        throw std::runtime_error("Steady error check steady message file");
    }
    wanda_input_file.close();
    wanda_output_file.close();
    std::string file = wanda_input_file.get_filename();
    auto sl = file.find_last_of('\\');
    if (sl > file.size())
    {
        sl = file.find_last_of('/');
    }

    std::string casename =
        wanda_input_file.get_filename().substr(sl + 1, wanda_input_file.get_filename().size() - 4 - sl - 1);
    std::string exe = wanda_bin + "unsteady.exe";
    std::string command_line = " \"" + wanda_input_file.get_filename() + "\" \"";

#ifdef _WINDOWS
    try
    {
        run_external_program_win(exe, command_line);
    }
    catch (std::exception &ex)
    {
        throw(ex);
    }
#else
    try
    {
        command_line = exe + command_line;
        const char *command = command_line.c_str();
        system(command);
    }
    catch (std::exception &ex)
    {
        throw(ex);
    }
#endif
    wanda_input_file.open();
    wanda_output_file.open();
    reload_component_indices();
    reload_output();
    load_unsteady_messages();
}

void wanda_model::reset_wdo_pointer()
{
    auto &ref_interval = get_property("Refresh interval");
    ref_interval.set_scalar(-10.0);
    ref_interval.set_modified(false);
    std::vector<int> value(1, -10);
    wanda_input_file.write_int_elements("CALC_CONTR_DATA", "Refresh_interval", nefis_file::single_elem_uindex, value);
}

void wanda_model::resume_unsteady_until(float simulation_time)
{
    auto &sim_time = get_property("Simulation time");
    auto dt = get_property("Time step").get_scalar_float();
    auto &start_time = get_property("Start time");
    start_time.set_scalar(sim_time.get_scalar_float() + dt);
    sim_time.set_scalar(simulation_time);
    sim_time.set_modified(false);
    start_time.set_modified(false);
    std::vector<float> value(4, simulation_time);
    wanda_input_file.write_float_elements("CALC_CONTR_DATA", "End_time", nefis_file::single_elem_uindex, value);
    std::vector<float> value2(4, sim_time.get_scalar_float() + dt);
    wanda_input_file.write_float_elements("CALC_CONTR_DATA", "Start_time", nefis_file::single_elem_uindex, value2);
    run_unsteady();
}

std::vector<std::vector<std::string>> wanda_model::get_case_units() const
{
    std::vector<std::vector<std::string>> list_unit;
    for (auto item : case_units)
    {
        std::vector<std::string> unit;
        unit.push_back(item.first);
        unit.push_back(item.second);
        list_unit.push_back(unit);
    }
    return list_unit;
}

std::vector<std::string> wanda_model::get_possible_dimensions(std::string unit) const
{
    if (unit_list.find(unit) != unit_list.end())
    {
        std::vector<std::string> units;
        for (auto unit_item : unit_list.at(unit))
        {
            units.push_back(unit_item.first);
        }
        return units;
    }
    throw std::invalid_argument(unit + " does not exist");
}

std::string wanda_model::get_current_dim(std::string unit_key)
{
    if (unit_list.find(unit_key) != unit_list.end())
    {
        return case_units[unit_key];
    }
    throw std::invalid_argument(unit_key + " does not exist");
}

void wanda_model::set_unit(std::string unit, std::string dimension)
{
    if (unit_list.find(unit) != unit_list.end())
    {
        std::vector<std::string> pos_units = get_possible_dimensions(unit);
        if (std::find(pos_units.begin(), pos_units.end(), dimension) != pos_units.end())
        {
            case_units[unit] = dimension;
            unit_group = "UNIT_GROUP_USER";
            set_unit_factors();
            return;
        }
        throw std::invalid_argument(dimension + " does not exist for " + unit);
    }
    throw std::invalid_argument(unit + " does not exist");
}

void wanda_model::switch_to_unit_SI()
{
    unit_group = "UNIT_GROUP_SI";
    case_units = component_definition->get_case_unit(unit_group);
    set_unit_factors();
}

void wanda_model::switch_to_unit_UK()
{
    unit_group = "UNIT_GROUP_UK";
    case_units = component_definition->get_case_unit(unit_group);
    set_unit_factors();
}

void wanda_model::switch_to_unit_US()
{
    unit_group = "UNIT_GROUP_US";
    case_units = component_definition->get_case_unit(unit_group);
    set_unit_factors();
}

void wanda_model::switch_to_unit_user()
{
    unit_group = "UNIT_GROUP_USER";
    std::vector<std::vector<std::string>> case_unit_descr(1, std::vector<std::string>(100));
    wanda_input_file.get_string_element("CASE_INFORMATION", "User_unit_descr", {1, 1, 1}, {1, 100, 1}, 16,
                                        case_unit_descr);
    std::vector<std::vector<std::string>> case_unit_dim(1, std::vector<std::string>(100));
    wanda_input_file.get_string_element("CASE_INFORMATION", "User_unit_dims", {1, 1, 1}, {1, 100, 1}, 12,
                                        case_unit_dim);

    if (case_unit_descr[0][0] != "")
    {
        for (int i = 0; i < case_unit_descr[0].size(); i++)
        {
            if (case_unit_descr[0][i] != "")
            {
                // from description to key
                //  auto test =
                //  unit_list[case_unit_dim[0][i]][case_unit_descr[0][i]].unit_description;
                case_units[case_unit_dim[0][i]] = case_unit_descr[0][i];
            }
        }
    }
    else
    {
        case_units = component_definition->get_case_unit(unit_group);
    }
    set_unit_factors();
}

void wanda_model::switch_to_unit_Wanda()
{
    unit_group = "UNIT_GROUP_WD";
    case_units = component_definition->get_case_unit(unit_group);
    set_unit_factors();
}

//! Generate vector with globvar values for HCS computation
/*!
* Generate vector with globvar values for HCS computation. This function is
* only used privately in wanda_model. The globvar values are stored in the wanda_model
* class, while the HCS are computed in the wanda_component classes.
\return std::vector with globvar values in standardized sequence
*/
std::vector<float> wanda_model::get_globvar_hcs()
{
    return {get_property("Density").get_scalar_float(),
            get_property("Gravitational acceleration").get_scalar_float(),
            0.0,
            get_property("Time step").get_scalar_float(),
            0.0,
            0.0,
            0.0,
            get_property("Bulk modulus").get_scalar_float(),
            get_property("Transient mode").get_scalar_float()};
}

void wanda_model::re_calculate_hcs()
{
    for (auto &comp : phys_components)
    {
        if (comp.second.get_num_hcs() != 0)
        {
            if (comp.second.is_modified() || glob_var_modified() && !comp.second.is_disused())
            {
                calc_hsc(comp.second);                
            }
        }
    }
}

void wanda_model::load_steady_messages()
{
    if (steady_message_not_loaded)
    {
        load_message("STEADY_MESSAGE");
        steady_message_not_loaded = false;
    }
}

void wanda_model::load_unsteady_messages()
{
    if (unsteady_message_not_loaded)
    {
        load_message("UNSTEADY_MESSAGE");
        unsteady_message_not_loaded = false;
    }
}

std::vector<float> wanda_model::get_time_steps() const
{
    if (simulation_time_steps.empty())
    {
        throw std::runtime_error("Output data not loaded");
    }
    return simulation_time_steps;
}


bool wanda_model::has_property(const std::string &description) const
{
    return global_vars.find(description) != global_vars.end() || mode_and_opt.find(description) != mode_and_opt.end();
}

bool wanda_model::component_exists(const std::string &comp_name) const
{
    for (auto &comp : phys_components)
    {
        if (comp.second.get_complete_name_spec() == comp_name)
        {
            return true;
        }
    }
    for (auto &comp : ctrl_components)
    {
        if (comp.second.get_complete_name_spec() == comp_name)
        {
            return true;
        }
    }
    return false;
}

bool wanda_model::component_exists(const wanda_component &comp) const
{
    if (phys_components.find(comp.get_key_as_string()) != phys_components.end())
    {
        return true;
    }
    if (ctrl_components.find(comp.get_key_as_string()) != ctrl_components.end())
    {
        return true;
    }
    return false;
}

bool wanda_model::node_exists(const std::string &node_name) const
{
    return name2_phys_node.contains(node_name);
}

bool wanda_model::node_exists(const wanda_node &node) const
{
    return phys_nodes.contains(node.get_key_as_string());
}

bool wanda_model::sig_line_exists(const std::string &comp_name) const
{ 
    for (auto &comp : signal_lines)
    {
        if (comp.second.get_complete_name_spec() == comp_name)
        {
            return true;
        }
    }
    return false;
}

bool wanda_model::sig_line_exists(const wanda_sig_line &sig_line) const
{
    if (signal_lines.find(sig_line.get_key_as_string()) != signal_lines.end())
    {
        return true;
    }

    return false;
}

void wanda_model::add_data_from_template(std::unordered_map<std::string, wanda_prop_template> data)
{
    for (auto item : data)
    {
        if (has_property(item.first))
        {
            auto &prop = get_property(item.first);
            prop.set_value_from_template(item);
        }
    }
}

void wanda_model::add_data_from_template_file(std::string template_file)
{
    auto data = wanda_helper_functions::load_template(template_file);
    add_data_from_template(data);
}

void wanda_model::upgrade_model()
{
    upgrade_wdi();
    save_model_input();
}

void wanda_model::upgrade_wdi()
{
    // check if data from untitled.wdi is newer then date from input file
    nefis_file untitled(component_definition->get_data_path() + "untitled.wdi");
    untitled.open('r');
    std::vector<std::string> data_untit(1);
    untitled.get_string_element("WANDA", "Date_time_wdf", nefis_file::single_elem_uindex, 17, data_untit);
    auto data_untit_int = parse_date_time(data_untit[0]);
    std::vector<std::string> data_case(1);
    wanda_input_file.get_string_element("WANDA", "Date_time_wdf", nefis_file::single_elem_uindex, 17, data_case);

    auto data_case_int = parse_date_time(data_case[0]);
    bool convert = false;
    // check if case needs to be converted is needed when data of untitled is newer then data of case
    for (int i = 0; i < data_untit_int.size(); i++)
    {
        if (data_untit_int[i] > data_case_int[i])
        {
            convert = true;
            break;
        }
    }
    if (!convert)
    {
        return;
    }
    // copy existing case to case_old and open it
    close();
    std::vector<std::string> extensions;
    extensions.emplace_back("wdi");
    extensions.emplace_back("wdo");
    extensions.emplace_back("wdx");
    auto casefull = wanda_input_file.get_filename();
    for (auto extension : extensions)
    {
        const std::filesystem::path from(casefull.substr(0, casefull.size() - 3) + extension);
        const std::filesystem::path to(casefull.substr(0, casefull.size() - 4) + "_old_version." + extension);
        try
        {
            if (FileExists(from.generic_string()))
            {
                std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing);
            }
        }
        catch (std::exception &e)
        {
            throw e.what();
        }
    }
    // create new wandacase in model
    wanda_model old_model(casefull.substr(0, casefull.size() - 4) + "_old_version.wdi",
                          component_definition->get_data_path());
    new_wanda_case(casefull);
    // copy all components from existing case to new case.
    old_model.upgrade_components();
    copy_all_from_model(old_model);
    // copying globvar and mode and option items;
    for (auto &global_var : global_vars)
    {
        auto old_globvar = old_model.get_property(global_var.first);
        if (global_var.second.has_table())
        {
            auto &table_new = global_var.second.get_table();
            auto table_old = old_globvar.get_table();
            for (auto description : table_new.get_descriptions())
            {
                table_new.set_float_column(description, table_old.get_float_column(description));
            }
            continue;
        }
        if (global_var.second.has_scalar())
        {
            global_var.second.set_scalar(old_globvar.get_scalar_float());
            continue;
        }
        if (global_var.second.has_string())
        {
            global_var.second.set_scalar(old_globvar.get_scalar_str());
            continue;
        }
    }
    for (auto &option : mode_and_opt)
    {
        auto old_option = old_model.get_property(option.first);
        if (option.second.has_scalar())
        {
            option.second.set_scalar(old_option.get_scalar_float());
        }
        if (option.second.has_string())
        {
            option.second.set_scalar(old_option.get_scalar_str());
        }
        if (option.second.has_table())
        {
            auto &table_new = option.second.get_table();
            auto table_old = old_option.get_table();
            for (auto description : table_new.get_descriptions())
            {
                table_new.set_float_column(description, table_old.get_float_column(description));
            }
        }
    }
}

// parse date time string from datetime_wdf to vector of int, year, month, day, hour, minute
std::vector<int> wanda_model::parse_date_time(std::string date_time_string)
{
    const std::array<std::string, 12> months = {
        std::string("JAN"), "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

    std::string year = date_time_string.substr(0, 4);
    std::string month = date_time_string.substr(5, 3);
    auto ind = std::find(months.begin(), months.end(), month);
    int index = static_cast<int>(std::distance(months.begin(), ind)) + 1;
    std::string day = date_time_string.substr(9, 2);
    std::string hour = date_time_string.substr(12, 2);
    std::string minute = date_time_string.substr(15, 2);

    // order of vector should be { year, month, day, hour, minute }
    std::vector<int> date_time_vector = {std::stoi(year), index, std::stoi(day), std::stoi(hour), std::stoi(minute)};
    return date_time_vector;
}


void wanda_model::upgrade_components()
{
    auto list_of_comps = get_all_components_str();
    for (auto &comp_name : list_of_comps)
    {
        auto comp = get_component(comp_name);
        if (comp.get_default_mask()[0] == 'X')
        {
            std::string type_name = component_definition->get_type_name_phys(comp.get_convert2component());
            auto &new_comp = add_component(type_name, comp.get_position());
            for (auto &prop : new_comp)
            {
                if (comp.contains_property(prop.second.get_description()))
                {
                    if (prop.second.has_table())
                    {
                        auto &table_old = comp.get_property(prop.second.get_description()).get_table();
                        auto &table_new = prop.second.get_table();
                        for (auto description : table_old.get_descriptions())
                        {
                            if (table_new.has_description(description))
                            {
                                if (table_old.is_string_column(description))
                                {
                                    table_new.set_string_column(description, table_old.get_string_column(description));
                                }
                                else
                                {
                                    table_new.set_float_column(description, table_old.get_float_column(description));
                                }
                            }
                        }
                    }
                    else if (prop.second.has_string() &&
                             comp.get_property(prop.second.get_description()).get_spec_status())
                    {
                        prop.second.set_scalar(comp.get_property(prop.second.get_description()).get_scalar_str());
                    }
                    else if (prop.second.has_scalar() &&
                             comp.get_property(prop.second.get_description()).get_spec_status())
                    {
                        prop.second.set_scalar(comp.get_property(prop.second.get_description()).get_scalar_float());
                    }
                }
            }
            new_comp.set_angle(comp.get_angle());
            new_comp.set_comment(comp.get_comment());
            for (auto keyword : comp.get_keywords())
            {
                new_comp.add_keyword(keyword);
            }
            new_comp.set_sequence_number(comp.get_sequence_number());
            new_comp.set_model_name(comp.get_model_name());
            new_comp.set_sequence_number(comp.get_sequence_number());
            if (new_comp.is_pipe())
            {
                new_comp.set_material_name(comp.get_material_name());
            }

            new_comp.set_ref_id(comp.get_ref_id());

            new_comp.set_disused(comp.is_disused());
            if (new_comp.has_action_table())
            {
                if (comp.has_action_table())
                {
                    new_comp.set_use_action_table(comp.is_action_table_used());
                }
                else
                {
                    new_comp.set_use_action_table(false);
                }
            }
            std::vector<wanda_node *> connected_nodes;
            std::vector<std::vector<wanda_component *>> connected_ctrl_comps;
            std::vector<std::vector<int>> connected_ctrl_comps_con_point;
            std::vector<std::vector<std::vector<std::string>>> all_keywords;
            std::vector<std::vector<bool>> all_disused;
            for (int i = 1; i <= comp.get_number_of_connnect_points(); i++)
            {
                if (comp.is_node_connected(i))
                {
                    connected_nodes.push_back(&comp.get_connected_node(i));
                }
                else
                {
                    connected_nodes.push_back(nullptr);
                }
                std::vector<wanda_component *> con_comps;
                std::vector<int> con_points;
                std::vector<std::vector<std::string>> keywords;
                std::vector<bool> disused;
                if (comp.is_sigline_connected(i, false))
                {
                    for (auto &sig_line : comp.get_connected_sigline(i, false))
                    {
                        con_comps.push_back(sig_line->get_input_component());
                        con_points.push_back(sig_line->get_input_connection_point());
                        keywords.push_back(sig_line->get_keywords());
                        disused.push_back(sig_line->is_disused());
                        delete_sig_line(*sig_line);
                    }
                }
                connected_ctrl_comps.push_back(con_comps);
                connected_ctrl_comps_con_point.push_back(con_points);
                all_keywords.push_back(keywords);
                all_disused.push_back(disused);
            }

            int i = comp.get_number_of_connnect_points() + 1;
            std::vector<wanda_component *> con_comps;
            std::vector<int> con_points;
            std::vector<std::vector<std::string>> keywords;
            std::vector<bool> disused;
            if (comp.is_sigline_connected(i, false))
            {
                for (auto sig_line : comp.get_connected_sigline(i, false))
                {
                    con_comps.push_back(sig_line->get_input_component());
                    con_points.push_back(sig_line->get_input_connection_point());
                    keywords.push_back(sig_line->get_keywords());
                    disused.push_back(sig_line->is_disused());
                    delete_sig_line(*sig_line);
                }
            }
            connected_ctrl_comps.push_back(con_comps);
            connected_ctrl_comps_con_point.push_back(con_points);
            all_keywords.push_back(keywords);
            all_disused.push_back(disused);
            con_comps.clear();
            con_points.clear();
            keywords.clear();
            disused.clear();

            if (comp.is_sigline_connected(i, true))
            {
                for (auto sig_line : comp.get_connected_sigline(i, true))
                {
                    con_comps.push_back(sig_line->get_output_component());
                    con_points.push_back(sig_line->get_output_connection_point());
                    keywords.push_back(sig_line->get_keywords());
                    disused.push_back(sig_line->is_disused());
                    delete_sig_line(*sig_line);
                }
            }
            connected_ctrl_comps.push_back(con_comps);
            connected_ctrl_comps_con_point.push_back(con_points);
            all_keywords.push_back(keywords);
            all_disused.push_back(disused);
            // changing the name of the new_component and the key and deleting the old component
            std::string name = comp.get_name();
            auto key = comp.get_key();
            delete_component(comp);
            name2_phys_comp.erase(new_comp.get_complete_name_spec());
            new_comp.set_name(name);
            // changing the component key to the orginal component key to ensure it works with the wdx file
            auto old_key = new_comp.get_key_as_string();
            new_comp.set_comp_key(key);
            auto comp_temp = phys_components.at(old_key);
            phys_components.erase(old_key);
            phys_components.erase(comp_temp.get_key_as_string());
            phys_components.emplace(comp_temp.get_key_as_string(), comp_temp);
            name2_phys_comp.emplace(comp_temp.get_complete_name_spec(),
                                    &phys_components.at(comp_temp.get_key_as_string()));
            wanda_component *new_comp2 = &phys_components.at(comp_temp.get_key_as_string());

            for (i = 0; i < new_comp2->get_number_of_connnect_points(); i++)
            {
                if (connected_nodes[i] != nullptr)
                {
                    connect(*new_comp2, i + 1, *connected_nodes[i]);
                }
                if (!connected_ctrl_comps[i].empty()) // can only be output (sensor)
                {
                    for (int j = 0; i < connected_ctrl_comps[i].size(); i++)
                    {
                        auto &sig_line = connect(*new_comp2, i + 1, *connected_ctrl_comps[i][j],
                                                 connected_ctrl_comps_con_point[i][j]);
                        sig_line.set_disused(all_disused[i][j]);
                        for (auto keyword : all_keywords[i][j])
                        {
                            sig_line.add_keyword(keyword);
                        }
                    }
                }
            }

            i = comp.get_number_of_connnect_points();
            if (!connected_ctrl_comps[i].empty()) // can only be output (sensor)
            {
                for (int j = 0; j < connected_ctrl_comps[i].size(); j++)
                {
                    auto &sig_line =
                        connect(*new_comp2, i + 1, *connected_ctrl_comps[i][j], connected_ctrl_comps_con_point[i][j]);
                    sig_line.set_disused(all_disused[i][j]);
                    for (auto keyword : all_keywords[i][j])
                    {
                        sig_line.add_keyword(keyword);
                    }
                }
            }
            i++;
            if (!connected_ctrl_comps[i].empty()) // can only be output (sensor)
            {
                for (int j = 0; j < connected_ctrl_comps[i].size(); j++)
                {
                    auto &sig_line =
                        connect(*connected_ctrl_comps[i][j], connected_ctrl_comps_con_point[i][j], *new_comp2, i);
                    sig_line.set_disused(all_disused[i][j]);
                    for (auto keyword : all_keywords[i][j])
                    {
                        sig_line.add_keyword(keyword);
                    }
                }
            }
        }
    }
}

void wanda_model::change_comp_type(const std::string &name, const std::string &type)
{
    auto &comp = get_component(name);
    auto &comp_new = add_component(type, comp.get_position());
    if ((component_definition->is_physical_component(comp.get_class_sort_key()) &
         component_definition->is_physical_component(comp.get_class_sort_key())) ||
        (component_definition->is_control_component(comp.get_class_sort_key()) &
         component_definition->is_control_component(comp.get_class_sort_key())))
    {
        // TODO  door de lijst van de oude component heen lopen, en die verplaatsen naar de nieuwe lijst!!
        for (auto &prop : comp_new)
        {
            if (comp.contains_property(prop.second.get_description()))
            {
                prop.second =
                    comp.get_property(prop.second.get_description()); // maakt kopie van property, maar tabellen en meuk
                                                                      // wordt dan niet goed meegenomen?
            }
        }
        if (component_definition->is_physical_component(comp.get_class_sort_key()))
        {
            for (int i = 1; i <= comp.get_number_of_connnect_points(); i++)
            {
                if (comp.is_node_connected(i))
                {
                    auto &node = comp.get_connected_node(i);
                    disconnect(comp, i);

                    if (i <= comp_new.get_number_of_connnect_points())
                    {
                        connect(comp_new, i, node);
                    }
                }
                if (comp.is_sigline_connected(i, false))
                {
                    for (auto &sigline : comp.get_connected_sigline(i, false))
                    {
                        sigline->connect_output(comp_new, i);
                    }
                }
            }
            if (comp.is_sigline_connected(comp.get_number_of_connnect_points() + 1, true))
            {
                if (comp_new.is_controlable() && comp_new.get_ctrl_input_type() == comp.get_ctrl_input_type())
                {
                    for (auto &sigline : comp.get_connected_sigline(comp.get_number_of_connnect_points() + 1, true))
                    {
                        sigline->connect_input(comp_new, comp.get_number_of_connnect_points() + 1);
                    }
                }
                else
                {
                    disconnect(comp, comp.get_number_of_connnect_points() + 1);
                }
            }
            if (comp.is_sigline_connected(comp.get_number_of_connnect_points() + 1, false))
            {
                for (auto &sigline : comp.get_connected_sigline(comp.get_number_of_connnect_points() + 1, false))
                {
                    sigline->connect_output(comp_new, comp_new.get_number_of_connnect_points() + 1);
                }
            }
            const auto comp_name = comp.get_name();
            delete_component(comp);
            comp_new.set_name(comp_name);
            name2_phys_comp.emplace(name, &phys_components.at(comp_new.get_key_as_string()));
        }
        else
        {
            for (int i = 1; i <= comp.get_num_input_channels(); i++)
            {
                if (i <= comp_new.get_num_input_channels())
                {
                    auto siglines = comp.get_connected_sigline(i, true);
                    for (auto sigline : siglines)
                    {
                        comp.disconnect(i, true);
                        sigline->connect_input(comp_new, i);
                        comp_new.connect(*sigline, i, true);
                    }
                }
            }
            for (int i = 1; i <= comp.get_num_output_channels(); i++)
            {
                if (i <= comp_new.get_num_output_channels())
                {
                    auto siglines = comp.get_connected_sigline(i, false);
                    for (auto sigline : siglines)
                    {
                        comp.disconnect(i, false);
                        sigline->connect_output(comp_new, i);
                        comp_new.connect(*sigline, i, false);
                    }
                }
            }
            const auto comp_name = comp.get_name();
            delete_component(comp);
            comp_new.set_name(comp_name);
        }
    }
    else
    {
        delete_component(comp_new);
        throw std::invalid_argument("Cannot convert control component to physical component or visa versa");
    }
}

void wanda_model::change_node_type(const std::string &name, const std::string &type)
{
    auto &node = get_node(name);
    auto &node_new = add_node(type, node.get_position());

    for (auto &prop : node_new)
    {
        if (node.contains_property(prop.second.get_description()))
        {
            prop.second = node.get_property(prop.second.get_description());
        }
    }

    for (auto comp : node.get_connected_components())
    {
        int con_point = comp->get_connect_point(node);
        comp->disconnect(node);
        node.disconnect(*comp);
        comp->connect(node_new, con_point);
        node_new.connect(*comp);
    }

    const auto node_name = node.get_name();
    delete_node(node);
    node_new.set_name(node_name);
    phys_nodes.emplace(node_new.get_key_as_string(), node_new);
    name2_phys_node.emplace(name, &phys_nodes.at(node_new.get_key_as_string()));
}

std::unordered_map<std::string, std::vector<std::string>> wanda_model::validate_model_input()
{
    std::unordered_map<std::string, std::vector<std::string>> results;
    for (auto &phys_component : phys_components)
    {
        if (!phys_component.second.is_disused())
        {
            auto message = phys_component.second.validate_input(get_view_mask());
            if (!message.empty())
            {
                results.emplace(phys_component.second.get_complete_name_spec(), message);
            }
        }
    }

    for (auto &component : ctrl_components)
    {
        if (!component.second.is_disused())
        {
            auto message = component.second.validate_input(get_view_mask());
            if (!message.empty())
            {
                results.emplace(component.second.get_complete_name_spec(), message);
            }
        }
    }
    for (auto &node : phys_nodes)
    {
        if (!node.second.is_disused())
        {
            auto message = node.second.validate_input(get_view_mask());
            if (!message.empty())
            {
                results.emplace(node.second.get_complete_name_spec(), message);
            }
        }
    }
    return results;
}

std::unordered_map<std::string, std::vector<int>> wanda_model::validate_connectivity()
{
    std::unordered_map<std::string, std::vector<int>> results;
    for (auto &component : phys_components)
    {
        if (!component.second.is_disused())
        {
            std::vector<int> temp_result;
            for (int i = 1; i <= component.second.get_number_of_connnect_points(); i++)
            {
                if (!component.second.is_node_connected(i))
                {
                    temp_result.push_back(i);
                }
            }
            if (!temp_result.empty())
            {
                results.emplace(component.second.get_complete_name_spec(), temp_result);
            }
        }
    }
    for (auto &node : phys_nodes)
    {
        if (!node.second.is_disused())
        {
            std::vector<int> temp_result;
            temp_result.push_back(0);
            if (node.second.get_connected_components().size() == 0)
            {
                results.emplace(node.second.get_complete_name_spec(), temp_result);
            }
        }
    }
    for (auto &component : ctrl_components)
    {
        if (!component.second.is_disused())
        {
            std::vector<int> temp_result;
            for (int i = 1; i <= component.second.get_num_input_channels(); i++)
            {
                if (component.second.get_min_input_channels(i) > 0)
                {
                    if (!component.second.is_sigline_connected(i, true))
                    {
                        temp_result.push_back(i);
                    }
                }
                if (!temp_result.empty())
                {
                    results.emplace(component.second.get_complete_name_spec(), temp_result);
                }
            }
        }
    }
    return results;
}

wanda_node &wanda_model::split_pipe(wanda_component &pipe, float loc)
{
    if (!component_exists(pipe))
    {
        throw(pipe.get_complete_name_spec() + " does not exist in Wanda model");
    }
    auto length = pipe.get_property("Length").get_scalar_float();
    if (loc >= length)
    {
        throw std::invalid_argument("Pipe length smaller than given location cannot split pipes");
    }
    auto prof_set = pipe.get_property("Geometry input").get_scalar_str();
    auto &node = pipe.get_connected_node(2);
    auto location = pipe.get_position();
    location[0] += 5;
    auto &new_pipe = add_component(&pipe, location);
    connect(new_pipe, 2, node);
    disconnect(pipe, 2);
    auto &item = connect(pipe, 2, new_pipe, 1);
    if (prof_set == "Length")
    {
        pipe.get_property("Length").set_scalar(loc);
        new_pipe.get_property("Length").set_scalar(length - loc);
        // wanda_node* new_node = dynamic_cast<wanda_node*>(&item);
        auto height = pipe.get_property("Profile").get_table().get_float_column("Height");
        auto node_el = (height[1] - height[0]) / length * loc + height[0];
        item.get_property("Elevation").set_scalar(node_el);
    }
    else if (prof_set == "l-h")
    {
        auto &tab = pipe.get_property("Profile").get_table();
        auto x = tab.get_float_column("X-distance");
        auto height = tab.get_float_column("Height");
        auto s = tab.get_float_column("S-distance");
        auto iter = std::lower_bound(s.begin(), s.end(), loc);
        int index = int(iter - s.begin());
        auto x_loc = x[index - 1] + (x[index] - x[index - 1]) / (s[index] - s[index - 1]) * (loc - s[index - 1]);
        auto h_loc =
            height[index - 1] + (height[index] - height[index - 1]) / (s[index] - s[index - 1]) * (loc - s[index - 1]);
        std::vector<float> x1;
        std::vector<float> h1;
        std::vector<float> s1;
        std::copy(x.begin(), x.begin() + index, std::back_inserter(x1));
        std::copy(height.begin(), height.begin() + index, std::back_inserter(h1));
        std::copy(s.begin(), s.begin() + index, std::back_inserter(s1));
        x1.push_back(x_loc);
        h1.push_back(h_loc);
        s1.push_back(loc);
        tab.resize_columns(x1.size());
        tab.set_float_column("X-distance", x1);
        tab.set_float_column("Height", h1);
        tab.set_float_column("S-distance", s1);        
        std::vector<float> x2;
        std::vector<float> h2;
        std::vector<float> s2;
        x2.push_back(x_loc);
        h2.push_back(h_loc);
        s2.push_back(loc);
        std::copy(x.begin() + index, x.end(), std::back_inserter(x2));
        std::copy(height.begin() + index, height.end(), std::back_inserter(h2));
        std::copy(s.begin() + index, s.end(), std::back_inserter(s2));
        auto &tab2 = new_pipe.get_property("Profile").get_table();
        tab2.set_float_column("X-distance", x2);
        tab2.set_float_column("Height", h2);
        tab2.set_float_column("S-distance", s2);
        item.get_property("Elevation").set_scalar(h_loc);
    }
    else if (prof_set == "xyz")
    {
        auto &tab = pipe.get_property("Profile").get_table();
        auto x = tab.get_float_column("X-abs");
        auto y = tab.get_float_column("Y-abs");
        auto z = tab.get_float_column("Z-abs");
        auto s = tab.get_float_column("S-distance");
        auto iter = std::lower_bound(s.begin(), s.end(), loc);
        int index = int(iter - s.begin());
        auto x_loc = x[index - 1] + (x[index] - x[index - 1]) / (s[index] - s[index - 1]) * (loc - s[index - 1]);
        auto y_loc = y[index - 1] + (y[index] - y[index - 1]) / (s[index] - s[index - 1]) * (loc - s[index - 1]);
        auto z_loc = z[index - 1] + (z[index] - z[index - 1]) / (s[index] - s[index - 1]) * (loc - s[index - 1]);
        std::vector<float> x1;
        std::vector<float> y1;
        std::vector<float> z1;
        std::copy(x.begin(), x.begin() + index, std::back_inserter(x1));
        std::copy(y.begin(), y.begin() + index, std::back_inserter(y1));
        std::copy(z.begin(), z.begin() + index, std::back_inserter(z1));
        x1.push_back(x_loc);
        y1.push_back(y_loc);
        z1.push_back(z_loc);
        tab.resize_columns(x1.size());
        tab.set_float_column("X-abs", x1);
        tab.set_float_column("Y-abs", y1);
        tab.set_float_column("Z-abs", z1);
        tab.set_float_column("X-distance", x1);
        tab.set_float_column("Height", x1);
        tab.set_float_column("S-distance", x1);
        std::vector<float> x2;
        std::vector<float> y2;
        std::vector<float> z2;
        x2.push_back(x_loc);
        y2.push_back(y_loc);
        z2.push_back(z_loc);
        std::copy(x.begin() + index, x.end(), std::back_inserter(x2));
        std::copy(y.begin() + index, y.end(), std::back_inserter(y2));
        std::copy(z.begin() + index, z.end(), std::back_inserter(z2));
        auto &tab2 = new_pipe.get_property("Profile").get_table();
        tab2.set_float_column("X-abs", x2);
        tab2.set_float_column("Y-abs", y2);
        tab2.set_float_column("Z-abs", z2);
        tab2.set_float_column("X-distance", x2);
        tab2.set_float_column("Height", x2);
        tab2.set_float_column("S-distance", x2);
        item.get_property("Elevation").set_scalar(z_loc);
    }
    else if (prof_set == "xyz dif")
    {
        auto &tab = pipe.get_property("Profile").get_table();
        auto x = tab.get_float_column("X-diff");
        auto y = tab.get_float_column("Y-diff");
        auto z = tab.get_float_column("Z-diff");
        auto s = tab.get_float_column("S-distance");
        auto height = tab.get_float_column("Height");
        auto iter = std::lower_bound(s.begin(), s.end(), loc);
        int index = int(iter - s.begin());
        auto x_loc = x[index] / (s[index] - s[index - 1]) * (loc - s[index - 1]);
        auto y_loc = y[index] / (s[index] - s[index - 1]) * (loc - s[index - 1]);
        auto z_loc = z[index] / (s[index] - s[index - 1]) * (loc - s[index - 1]);
        auto h_loc =
            height[index - 1] + (height[index] - height[index - 1]) / (s[index] - s[index - 1]) * (loc - s[index - 1]);
        std::vector<float> x1;
        std::vector<float> y1;
        std::vector<float> z1;
        std::copy(x.begin(), x.begin() + index, std::back_inserter(x1));
        std::copy(y.begin(), y.begin() + index, std::back_inserter(y1));
        std::copy(z.begin(), z.begin() + index, std::back_inserter(z1));
        x1.push_back(x_loc);
        y1.push_back(y_loc);
        z1.push_back(z_loc);
        tab.resize_columns(x1.size());
        tab.set_float_column("X-diff", x1);
        tab.set_float_column("Y-diff", y1);
        tab.set_float_column("Z-diff", z1);
        tab.set_float_column("X-distance", x1);
        tab.set_float_column("Height", x1);
        tab.set_float_column("S-distance", x1);
        std::vector<float> x2;
        std::vector<float> y2;
        std::vector<float> z2;
        x2.push_back(0.0);
        y2.push_back(0.0);
        z2.push_back(h_loc);
        x2.push_back(x[index] - x_loc);
        y2.push_back(y[index] - y_loc);
        z2.push_back(z[index] - z_loc);
        std::copy(x.begin() + index + 1, x.end(), std::back_inserter(x2));
        std::copy(y.begin() + index + 1, y.end(), std::back_inserter(y2));
        std::copy(z.begin() + index + 1, z.end(), std::back_inserter(z2));
        auto &tab2 = new_pipe.get_property("Profile").get_table();
        tab2.set_float_column("X-diff", x2);
        tab2.set_float_column("Y-diff", y2);
        tab2.set_float_column("Z-diff", z2);
        tab2.set_float_column("X-distance", x2);
        tab2.set_float_column("Height", x2);
        tab2.set_float_column("S-distance", x2);
        item.get_property("Elevation").set_scalar(h_loc);
    }
    //pipe.get_property("Length").set_scalar(loc);
    //new_pipe.get_property("Length").set_scalar(length - loc);
    calc_hsc(pipe);
    calc_hsc(new_pipe);
    return get_node(item.get_complete_name_spec());
}

void wanda_model::merge_pipes(wanda_component &pipe1, wanda_component &pipe2, int option)
{
    if (!component_exists(pipe1))
    {
        throw(pipe1.get_complete_name_spec() + " does not exist in Wanda model");
    }
    if (!component_exists(pipe1))
    {
        throw(pipe2.get_complete_name_spec() + " does not exist in Wanda model");
    }
    auto val_pipe1 = pipe1.validate_input(get_view_mask());
    if (val_pipe1.size() != 0)
    {
        throw std::runtime_error(pipe1.get_complete_name_spec() + " not all inputs are filled in");
    }
    auto val_pipe2 = pipe2.validate_input(get_view_mask());
    if (val_pipe2.size() != 0)
    {
        throw std::runtime_error(pipe2.get_complete_name_spec() + " not all inputs are filled in");
    }
    // ensuring the hcs are filled
    calc_hsc(pipe1);
    calc_hsc(pipe2);
    if (!pipe1.is_pipe())
    {
        throw std::invalid_argument(pipe1.get_complete_name_spec() + " is not a pipe");
    }
    if (!pipe2.is_pipe())
    {
        throw std::invalid_argument(pipe2.get_complete_name_spec() + " is not a pipe");
    }
    std::string node11;
    std::string node12;
    std::string node21;
    std::string node22;
    if (pipe1.is_node_connected(1))
    {
        node11 = pipe1.get_connected_node(1).get_complete_name_spec();
    }
    if (pipe2.is_node_connected(1))
    {
        node21 = pipe2.get_connected_node(1).get_complete_name_spec();
    }
    if (pipe1.is_node_connected(1))
    {
        node12 = pipe1.get_connected_node(2).get_complete_name_spec();
    }
    if (pipe2.is_node_connected(1))
    {
        node22 = pipe2.get_connected_node(2).get_complete_name_spec();
    }
    int con_point1 = 0;
    int con_point2 = 0;
    if (node11 == node21 && !node11.empty() && !node21.empty())
    {
        con_point1 = 1;
        con_point2 = 1;
    }
    if (node11 == node22 && !node11.empty() && !node22.empty())
    {
        con_point1 = 1;
        con_point2 = 2;
    }
    if (node12 == node21 && !node12.empty() && !node21.empty())
    {
        con_point1 = 2;
        con_point2 = 1;
    }
    if (node12 == node22 && !node12.empty() && !node22.empty())
    {
        con_point1 = 2;
        con_point2 = 2;
    }
    if (con_point1 == 0)
    {
        throw std::invalid_argument(pipe1.get_complete_name_spec() + " and " + pipe2.get_complete_name_spec() +
                                    " are not directly connected");
    }

    auto &node = pipe1.get_connected_node(con_point1);
    auto comps = node.get_connected_components();
    auto elevation = node.get_property("Elevation").get_scalar_float();
    std::vector<wanda_component *> sensorlist_node;
    if (comps.size() > 2)
    {
        // it might be control components
        for (auto &component : comps)
        {
            if (component->get_complete_name_spec() != pipe1.get_complete_name_spec() &&
                component->get_complete_name_spec() != pipe2.get_complete_name_spec())
            {
                if (component->get_item_type() == wanda_type::physical)
                {
                    throw std::invalid_argument(
                        node.get_complete_name_spec() + " is also connect to other components then " +
                        pipe1.get_complete_name_spec() + " and " + pipe2.get_complete_name_spec());
                }
                sensorlist_node.push_back(component);
            }
        }
    }

    // setting the input correct

    auto length1 = pipe1.get_property("Length").get_scalar_float();
    auto length2 = pipe2.get_property("Length").get_scalar_float();
    auto area1 = pipe1.get_area();
    auto area2 = pipe2.get_area();

    auto wave_speed1 = pipe1.get_property("Wave speed").get_scalar_float();
    auto wave_speed2 = pipe2.get_property("Wave speed").get_scalar_float();
    // length + profile
    auto setting_prof1 = pipe1.get_property("Geometry input").get_scalar_float();
    auto setting_prof2 = pipe2.get_property("Geometry input").get_scalar_float();
    // chaning the profile setting to at least l-h table
    pipe1.change_profile_tab(
        pipe1.get_property("Geometry input").get_list_item(max(max(setting_prof1, setting_prof2), 2)),
        get_globvar_hcs());
    pipe2.change_profile_tab(
        pipe2.get_property("Geometry input").get_list_item(max(max(setting_prof1, setting_prof2), 2)),
        get_globvar_hcs());
    delete_node(node);
    if (pipe1.get_property("Geometry input").get_scalar_str() == "Length")
    {
        pipe1.get_property("Length").set_scalar(length1 + length2);
    }
    if (pipe1.get_property("Geometry input").get_scalar_str() == "l-h")
    {
        // TODO check direction and combine
        // TODO correct for non zero start point
        auto &table1 = pipe1.get_property("Profile").get_table();
        auto &table2 = pipe2.get_property("Profile").get_table();
        auto l1 = table1.get_float_column("X-distance");
        auto h1 = table1.get_float_column("Height");
        auto l2 = table2.get_float_column("X-distance");
        auto h2 = table2.get_float_column("Height");

        // normal connection tables can be added together
        // removing the last point from the table, since this will be a duplicate
        if (con_point1 == 2 && con_point2 == 1)
        {
            auto l1last = l1.back();
            auto l2start = l2[0];
            l1.pop_back();
            h1.pop_back();
            for (int i = 0; i < l2.size(); i++)
            {
                l1.push_back(l2[i] + l1last - l2start);
                h1.push_back(h2[i]);
            }
        }
        if (con_point1 == 2 && con_point2 == 2)
        {
            auto l1last = l1.back();

            l1.pop_back();
            h1.pop_back();
            std::reverse(l2.begin(), l2.end());
            std::reverse(h2.begin(), h2.end());
            auto l2start = l2[0];
            for (int i = 0; i < l2.size(); i++)
            {
                l1.push_back(l2start - l2[i] + l1last);
                h1.push_back(h2[i]);
            }
        }
        if (con_point1 == 1 && con_point2 == 2)
        {
            std::reverse(l1.begin(), l1.end());
            std::reverse(h1.begin(), h1.end());
            auto l1start = l1[0];
            for (int i = 0; i < l1.size(); i++)
            {
                l1[i] = l1start - l1[i];
            }
            auto l1last = l1.back();
            l1.pop_back();
            h1.pop_back();
            std::reverse(l2.begin(), l2.end());
            std::reverse(h2.begin(), h2.end());
            auto l2start = l2[0];
            for (int i = 0; i < l2.size(); i++)
            {
                l1.push_back(l2start - l2[i] + l1last);
                h1.push_back(h2[i]);
            }
        }
        if (con_point1 == 1 && con_point2 == 1)
        {
            std::reverse(l1.begin(), l1.end());
            std::reverse(h1.begin(), h1.end());
            auto l1start = l1[0];
            for (int i = 0; i < l1.size(); i++)
            {
                l1[i] = l1start - l1[i];
            }
            auto l1last = l1.back();
            l1.pop_back();
            h1.pop_back();

            auto l2start = l2[0];
            for (int i = 0; i < l2.size(); i++)
            {
                l1.push_back(l2[i] + l1last - l2start);
                h1.push_back(h2[i]);
            }
        }

        table1.set_float_column("X-distance", l1);
        table1.set_float_column("Height", h1);
    }
    if (pipe1.get_property("Geometry input").get_scalar_str() == "xyz")
    {
        // TODO check direction and combine
        // TODO correct for non zero start point
        auto &table1 = pipe1.get_property("Profile").get_table();
        auto &table2 = pipe2.get_property("Profile").get_table();
        auto x1 = table1.get_float_column("X-abs");
        auto y1 = table1.get_float_column("Y-abs");
        auto z1 = table1.get_float_column("Z-abs");

        auto x2 = table2.get_float_column("X-abs");
        auto y2 = table2.get_float_column("Y-abs");
        auto z2 = table2.get_float_column("Z-abs");
        float x = x1.back();
        float y = y1.back();
        float z = z1.back();
        for (int i = 1; i < x2.size(); i++)
        {
            x1.push_back(x + x2[i] - x2[0]);
            y1.push_back(y + y2[i] - y2[0]);
            z1.push_back(z + z2[i]);
        }
        table1.set_float_column("X-abs", x1);
        table1.set_float_column("Y-abs", y1);
        table1.set_float_column("Z-abs", z1);
        // filling also the other tables so the length is correct these will be overwritten in calc_hcs
        table1.set_float_column("X-distance", z1);
        table1.set_float_column("Height", z1);
        table1.set_float_column("X-abs", x1);
        table1.set_float_column("Y-abs", y1);
        table1.set_float_column("Z-abs", z1);
    }
    if (pipe1.get_property("Geometry input").get_scalar_str() == "xyz dif")
    {
        // TODO check direction and combine
        // TODO correct for non zero start point
        auto &table1 = pipe1.get_property("Profile").get_table();
        auto &table2 = pipe2.get_property("Profile").get_table();
        auto x1 = table1.get_float_column("X-diff");
        auto y1 = table1.get_float_column("Y-diff");
        auto z1 = table1.get_float_column("Z-diff");
        auto x2 = table2.get_float_column("X-diff");
        auto y2 = table2.get_float_column("Y-diff");
        auto z2 = table2.get_float_column("Z-diff");
        x1.insert(x1.end(), x2.begin() + 1, x2.end());
        y1.insert(y1.end(), y2.begin() + 1, y2.end());
        z1.insert(z1.end(), z2.begin() + 1, z2.end());
        table1.set_float_column("X-diff", x1);
        table1.set_float_column("Y-diff", y1);
        table1.set_float_column("Z-diff", z1);
        // filling also the other tables so the length is correct these will be overwritten in calc_hcs
        table1.set_float_column("X-distance", x1);
        table1.set_float_column("S-distance", x1);
        table1.set_float_column("Height", x1);
        table1.set_float_column("X-abs", x1);
        table1.set_float_column("Y-abs", y1);
        table1.set_float_column("Z-abs", z1);
    }
    calc_hsc(pipe1);
    // inner diameter
    auto area = (area1 * length1 + area2 * length2) / (length1 + length2);
    static float pi = acos(-1);
    auto id = sqrt(area / pi * 4.0);
    pipe1.get_property("Inner diameter").set_scalar(id);
    // Wave speed
    float wavespeed = 0;
    if (option == 0)
    {
        float g = get_property("Gravitational acceleration").get_scalar_float();
        float watham1 = g * area1 * length1 / pow(wave_speed1, 2);
        float watham2 = g * area2 * length2 / pow(wave_speed2, 2);
        wavespeed = sqrt(9.81 * area * pipe1.get_property("Length").get_scalar_float() / (watham1 + watham2));
    }
    else if (option == 1)
    {
        float T1 = 2 * length1 / wave_speed1;
        float T2 = 2 * length2 / wave_speed2;
        wavespeed = (2 * pipe1.get_property("Length").get_scalar_float()) / (T1 + T2);
    }
    else
    {
        throw std::invalid_argument(std::to_string(option) + " is not a valid option");
    }
    if (wavespeed != 0)
    {
        pipe1.get_property("Wave speed mode").set_scalar("Specified");
        pipe1.get_property("Specified wave speed").set_scalar(wavespeed);
    }
    // deleting pipe2
    auto &node2 = pipe2.get_connected_node((con_point2 == 1) ? 2 : 1);

    // sensor connected to opsite point needs to move the con_point1 from pipe1
    if (pipe2.is_sigline_connected(con_point2 == 1 ? 2 : 1, false))
    {
        for (auto con_sig_line : pipe2.get_connected_sigline(con_point2 == 1 ? 2 : 1, false))
        {
            auto comp = con_sig_line->get_input_component();
            disconnect(*comp, 1, true);
            connect(pipe1, con_point1, *comp, 1);
        }
    }
    if (pipe2.is_sigline_connected(con_point2 == 1 ? 1 : 2, false))
    {
        for (auto con_sig_line : pipe2.get_connected_sigline(con_point2 == 1 ? 1 : 2, false))
        {
            auto comp = con_sig_line->get_input_component();
            disconnect(*comp, 1, true);
            connect(pipe1, 3, *comp, 1);
            comp->get_property("Meas location").set_scalar(con_point1 == 2 ? length1 : length2);
        }
    }

    if (pipe1.is_sigline_connected(con_point1 == 1 ? 1 : 2, false))
    {
        for (auto con_sig_line : pipe1.get_connected_sigline(con_point1 == 1 ? 1 : 2, false))
        {
            auto comp = con_sig_line->get_input_component();
            disconnect(*comp, 1, true);
            connect(pipe1, 3, *comp, 1);
            comp->get_property("Meas location").set_scalar(con_point1 == 2 ? length1 : length2);
        }
    }
    if (pipe2.is_sigline_connected(3, false))
    {
        for (auto sigline : pipe2.get_connected_sigline(3, false))
        {
            auto comp = sigline->get_input_component();
            disconnect(*comp, 1, true);
            connect(pipe1, 3, *comp, 1);
            auto meas_loc = comp->get_property("Meas location").get_scalar_float();
            comp->get_property("Meas location").set_scalar(meas_loc + (con_point1 == 2 ? length1 : length2));
        }
    }

    delete_component(pipe2);
    connect(pipe1, con_point1, node2);
    for (auto sensor : sensorlist_node)
    {
        connect(pipe1, 3, *sensor, 1);
        sensor->get_property("Meas location").set_scalar(con_point1 == 2 ? length1 : length2);
        // change the setting to the correct property!
    }
}

void wanda_model::switch_to_transient_mode()
{
    get_property("Engineering mode").set_scalar(0.0);
    get_property("Transient mode").set_scalar(1.0);
    get_property("Cavitation").set_scalar(1.0);
}

void wanda_model::switch_to_engineering_mode()
{
    get_property("Engineering mode").set_scalar(1.0);
    get_property("Transient mode").set_scalar(0.0);
    get_property("Cavitation").set_scalar(0.0);
}

std::vector<std::string> wanda_model::split_keyword_list(std::string keywords)
{
    // check if there are quotation marks, if so split them on the quotation marks else on the spaces
    auto ind = std::find(keywords.begin(), keywords.end(), '"');
    char delim = ' ';
    if (ind != keywords.end())
    {
        delim = '"';
    }
    std::vector<std::string> elems = wanda_helper_functions::split(keywords, delim);

    std::vector<std::string> results;
    for (auto result : elems)
    {
        if (result == " " || result.empty())
            continue;
        results.push_back(result);
    }
    return results;
}

std::string wanda_model::keywords2_list(std::vector<std::string> keyword_list)
{
    std::string delim = "";
    for (auto keyword : keyword_list)
    {
        auto ind = std::find(keyword.begin(), keyword.end(), ' ');
        if (ind != keyword.end())
        {
            delim = "\"";
            break;
        }
    }
    std::string result;
    for (auto keyword : keyword_list)
    {
        result += delim + keyword + delim;
        result += ' ';
    }
    return result.substr(0, result.size() - 1);
}

void wanda_model::change_component_name(wanda_component &comp, std::string new_name)
{
    // check if component exist in case either as hydraulic or control
    if (!component_exists(comp))
    {
        throw std::invalid_argument(comp.get_complete_name_spec() + " does not exist in model");
    }
    // remove name from name_2_phys name
    if (name2_phys_comp.find(comp.get_name_prefix() + " " + new_name) == name2_phys_comp.end())
    {
        name2_phys_comp.erase(name2_phys_comp.find(comp.get_complete_name_spec()));
        name2_phys_comp.emplace(comp.get_name_prefix() + new_name, &phys_components.at(comp.get_key_as_string()));

        //
        // changes name of component
        comp.set_name(new_name);
        return;
    }

    throw std::invalid_argument("Component with name " + new_name + " already exists in model");
}

void wanda_model::change_node_name(wanda_node &node, std::string new_name)
{
    // check if component exist in case either as hydraulic or control
    if (!node_exists(node))
    {
        throw std::invalid_argument(node.get_complete_name_spec() + " does not exist in model");
    }
    // remove name from name_2_phys name
    if (name2_phys_node.find(node.get_name_prefix() + " " + new_name) == name2_phys_node.end())
    {
        name2_phys_node.erase(name2_phys_node.find(node.get_complete_name_spec()));
        name2_phys_node.emplace(node.get_name_prefix() + new_name, &phys_nodes.at(node.get_key_as_string()));
        //
        // changes name of component
        node.set_name(new_name);
        return;
    }

    throw std::invalid_argument("Node with name " + node.get_name_prefix() + " " + new_name +
                                " already exists in model");
}

std::vector<wanda_property *> wanda_model::get_all_properties()
{
    std::vector<wanda_property *> list;
    for (auto &item : global_vars)
    {
        list.push_back(&item.second);
    }
    for (auto &item : mode_and_opt)
    {
        list.push_back(&item.second);
    }
    return list;
}

std::vector<std::string> wanda_model::get_all_properties_string()
{
    std::vector<std::string> list;
    for (auto &item : global_vars)
    {
        list.push_back(item.first);
    }
    for (auto &item : mode_and_opt)
    {
        list.push_back(item.first);
    }
    return list;
}

int wanda_model::get_element_size_wdi(std::string &element)
{
    return wanda_input_file.get_element_size(element);
}

int wanda_model::get_element_size_def(std::string &element)
{
    return component_definition->get_element_size(element);
}

std::vector<std::string> wanda_model::get_all_keywords() const
{
    std::set<std::string> keywords;
    for (auto comp : phys_components)
    {
        for(auto keyword : comp.second.get_keywords())
        {
            keywords.emplace(keyword);
        }
    }
    for (auto comp : ctrl_components)
    {
        for (auto keyword : comp.second.get_keywords())
        {
            keywords.emplace(keyword);
        }
    }
    for (auto comp : phys_nodes)
    {
        for (auto keyword : comp.second.get_keywords())
        {
            keywords.emplace(keyword);
        }
    }
    for (auto comp : signal_lines)
    {
        for (auto keyword : comp.second.get_keywords())
        {
            keywords.emplace(keyword);
        }
    }
    std::vector<std ::string> result(keywords.begin(), keywords.end());
    return result;
}

// float offset_x and float offset_y both have default value = 0.0
void wanda_model::copy_all_from_model(wanda_model &model, float offset_x, float offset_y)
{
    float offset_x_ = 0.0;
    if (offset_x > 1e-6)
    {
        offset_x_ = offset_x;
    }

    // find the minimum y position of the current model
    float offset_y_ = 0.0;
    if (offset_y > 1e-6)
    {
        offset_y_ = offset_y;
    }
    else
    {
        float min_y = 0.0;
        for (auto &comp : phys_components)
        {
            std::vector<float> pos = comp.second.get_position();
            if (min_y == 0)
                min_y = pos[1];
            if (pos[1] < min_y)
                min_y = pos[1];
        }
        offset_y_ = min_y - 10;
    }
    // adding all the components to the model and adjusting the position
    std::unordered_map<std::string, wanda_component *> comps_name_change;
    for (auto &comp : model.get_all_components())
    {
        std::vector<float> pos = comp->get_position();
        pos[0] += offset_x_;
        pos[1] += offset_y_;
        auto &new_comp = add_component(comp, pos);
        new_comp.set_new(true);
        new_comp.set_modified(true);
        new_comp.set_disused((comp->is_disused()));
        for (auto &item : new_comp)
        {
            item.second.set_modified(true);
            if (item.second.has_table())
            {
                for (auto description : item.second.get_table().get_descriptions())
                {
                    item.second.get_table().set_key(description, unref);
                }
            }
        }
        comps_name_change.emplace(comp->get_complete_name_spec(), &new_comp);
    }
    // adding nodes
    for (auto &node : model.get_all_nodes())
    {
        std::vector<float> pos = node->get_position();
        pos[0] += offset_x_;
        pos[1] += offset_y_;
        auto &new_node = add_node(node, pos);
        new_node.set_new(true);
        new_node.set_modified(true);
        new_node.set_disused(node->is_disused());
        for (auto &item : new_node)
        {
            item.second.set_modified(true);
        }

        for (auto &comp : node->get_connected_components())
        {
            auto con_comp = comps_name_change.at(comp->get_complete_name_spec());
            connect(*con_comp, comp->get_connect_point(*node), new_node);
        }
    }
    // adding signal lines
    for (auto sig_line : model.get_all_signal_lines())
    {
        // get input component
        std::string inname = sig_line->get_input_component()->get_complete_name_spec();
        auto incomp = comps_name_change.at(inname);
        int con_point_in = sig_line->get_input_connection_point();
        // get output component
        std::string outname = sig_line->get_output_component()->get_complete_name_spec();
        auto &outcomp = comps_name_change.at(outname);
        int con_point_out = sig_line->get_output_connection_point();
        auto &new_sig_line_temp = connect(*outcomp, con_point_out, *incomp, con_point_in);
        for (auto keyword : sig_line->get_keywords())
        {
            new_sig_line_temp.add_keyword(keyword);
        }
        auto new_sig_line = signal_lines.at(new_sig_line_temp.get_key_as_string());
        new_sig_line.set_disused(sig_line->is_disused());
        if (signal_lines.find(sig_line->get_key_as_string()) == signal_lines.end())
        {
            signal_lines.erase(new_sig_line.get_key_as_string());
            new_sig_line.set_comp_key(sig_line->get_key());
            signal_lines.emplace(new_sig_line.get_key_as_string(), new_sig_line);
            auto &sig_line_point = signal_lines.at(new_sig_line.get_key_as_string());
            incomp->disconnect(con_point_in, true);
            outcomp->disconnect(con_point_out, false);
            incomp->connect(sig_line_point, con_point_in, true);
            outcomp->connect(sig_line_point, con_point_out, false);
        }
    }
}

// private method
void wanda_model::reset_modified()
{
    _modified = false;
    for (auto &comp : phys_components)
    {
        comp.second.reset_modified();
    }
    for (auto &comp : ctrl_components)
    {
        comp.second.reset_modified();
    }
    for (auto &node : phys_nodes)
    {
        node.second.reset_modified();
    }
    for (auto &line : signal_lines)
    {
        line.second.reset_modified();
    }
    for (auto &globvar : global_vars)
    {
        globvar.second.set_modified(false);
    }
}

// private method
void wanda_model::read_general_items()
{
    // get the unit list
    unit_list = component_definition->get_unit_list();
    // read case units
    std::vector<std::string> un_group(1);
    wanda_input_file.get_string_element("CASE_INFORMATION", "Unit_group", nefis_file::single_elem_uindex, 16, un_group);
    unit_group = un_group[0];
    // unit list gets broken here somewhere!!
    if (unit_group.compare("UNIT_GROUP_USER") == 0)
    {
        switch_to_unit_user();
    }
    else
    {
        case_units = component_definition->get_case_unit(unit_group);
    }
    get_globvar_data();
    read_mode_and_options();
}

// private method
void wanda_model::get_globvar_data()
{
    globvar_base_data globvar;
    auto choice_lists = globvar.get_choice_lists();
    // todo add strings
    for (auto element : globvar.get_data_as_list())
    {
        if (element.type != "REAL" && element.type != "INTEGER" && element.type != "TABLE" && element.type != "CHOICE")
        {
            continue;
        }
        wanda_property prop;
        prop.set_wdo_postfix("GLOV");
        prop.settype();
        // check if the element type is a real then read it from the file and save
        // it to the globvar.
        if (element.type == "REAL")
        {
            std::vector<float> fltResult(element.elsize);
            wanda_input_file.get_float_element(element.groupname, element.element_name, nefis_file::single_elem_uindex,
                                               fltResult);
            prop.set_scalar(fltResult[element.elnum - 1]);
            prop.set_property_spec_inp_fld('R');
            if (element.unit_description != "")
            {
                prop.set_unit_dim(element.unit_description);
                prop.set_unit_factor(unit_list, case_units);
            }
        }
        if (element.type == "INTEGER")
        {
            std::vector<int> intResult(element.elsize);
            wanda_input_file.get_int_element(element.groupname, element.element_name, nefis_file::single_elem_uindex,
                                             intResult);
            prop.set_scalar(float(intResult[element.elnum - 1]));
            prop.set_property_spec_inp_fld('R');
        }
        if (element.type == "CHOICE")
        {
            std::vector<int> intResult(element.elsize);
            wanda_input_file.get_int_element(element.groupname, element.element_name, nefis_file::single_elem_uindex,
                                             intResult);
            prop.set_scalar(float(intResult[element.elnum - 1]));
            prop.set_property_spec_inp_fld('C');
            prop.set_list(choice_lists.at(element.name));
        }

        if (element.type == "TABLE" && element.name == "Temp.dep. properties")
        { // loading the fluid dependent fluid props
            wanda_property *prop_pointer = &prop;
            if (global_vars.find(element.name) != global_vars.end())
            {
                prop_pointer = &global_vars.at(element.name);
            }
            prop_pointer->set_property_spec_inp_fld('N');
            auto &table = prop_pointer->get_table();
            std::vector<std::string> table_key(element.elsize);
            wanda_input_file.get_string_element(element.groupname, element.element_name, nefis_file::single_elem_uindex,
                                                8, table_key);
            // loading table description to be used here

            auto description = globvar.get_temp_dep_fluid_descr().at(element.element_name);
            table.add_column(description, element.unit_description, table_key[element.elnum - 1], 'N', 1, 0, "", 'r');
            read_table(table);
        }

        prop.set_description(element.name);
        global_vars.emplace(element.name, prop);
    }
}

// private method
void wanda_model::read_mode_and_options()
{
    // TODO change to bit shift operation see write_mode_and_option
    std::vector<int> mode_and_opt_in(1);
    wanda_input_file.get_int_element("CASE_INFORMATION", "View_mask", nefis_file::single_elem_uindex, mode_and_opt_in);
    std::vector<bool> mode_option(28);
    int val = pow(2, 27);
    for (int i = 27; i >= 0; i--)
    {
        if (mode_and_opt_in[0] - val >= 0)
        {
            mode_option[i] = true;
            mode_and_opt_in[0] -= val;
            if (mode_and_opt_in[0] == 0)
            {
                break;
            }
        }
        val /= 2;
    }
    const mode_and_options_base_data mode_and_opt_list;

    for (auto item : mode_and_opt_list.get_mode_and_option_list())
    {
        wanda_property prop;
        prop.set_scalar(mode_option[item.bitmask_exponent]);
        prop.set_index(item.bitmask_exponent);
        prop.set_group_index(item.bitmask_exponent);
        prop.set_wdo_postfix("MAO");
        prop.settype();
        prop.set_scalar(mode_option[item.bitmask_exponent]);
        mode_and_opt.emplace(item.name, prop);
    }
}

int wanda_model::get_view_mask()
{
    int view_mask = 0;
    mode_and_options_base_data mode_and_opt_list;
    std::vector<bool> mode_option(28);
    for (auto item : mode_and_opt_list.get_mode_and_option_list())
    {
        float value = mode_and_opt[item.name].get_scalar_float();
        if (value == 1.0)
        {
            view_mask |= 1 << mode_and_opt[item.name].get_group_index();
        }
    }
    return view_mask;
}

// private method
void wanda_model::save_mode_and_options()
{
    std::vector<int> mode_and_opt_in(1);
    mode_and_opt_in[0] = get_view_mask();
    wanda_input_file.write_int_elements("CASE_INFORMATION", "View_mask", nefis_file::single_elem_uindex,
                                        mode_and_opt_in);
}

// private method
void wanda_model::read_nodes()
{
    number_physical_nodes = 0;
    int numrecords = wanda_input_file.get_maxdim_index("H_NODES");
    if (numrecords <= 0)
    {
        return;
    }
    const nefis_uindex uindex = {1, numrecords, 1};
    std::vector<std::string> Class_name(numrecords);
    wanda_input_file.get_string_element("H_NODES", "Class_name", uindex, 8, Class_name);
    std::vector<std::string> Name(numrecords);
    wanda_input_file.get_string_element("H_NODES", "Name", uindex, 0, Name);
    std::vector<std::string> H_comp_key(numrecords);
    wanda_input_file.get_string_element("H_NODES", "H_node_key", uindex, 8, H_comp_key);
    std::vector<std::string> KeywordsList(numrecords);
    wanda_input_file.get_string_element("H_NODES", "Keywords", uindex, 50, KeywordsList);
    std::vector<int> isDisused(numrecords);
    wanda_input_file.get_int_element("H_NODES", "Is_disused", {1, numrecords, 1}, isDisused);
    std::vector<float> abs_pos(2 * numrecords);
    wanda_input_file.get_float_element("H_NODES", "Abs_position", {1, numrecords, 1}, abs_pos);
    for (int i = 0; i < numrecords; i++)
    {
        auto nodekey = H_comp_key[i];
        if (nodekey == "Free")
        {
            continue;
        }
        int key = strtol(nodekey.substr(1).c_str(), nullptr, 10);
        auto *node = component_definition->get_node(Class_name[i]);

        phys_nodes.emplace(nodekey, *node);
        phys_nodes[nodekey].set_name(Name[i]);
        phys_nodes[nodekey].set_comp_key(key);
        // phys_nodes.emplace(nodekey, wanda_node(key, Class_name[i], Class_name[i], Name_prefix, Name[i], type_name));
        number_physical_nodes++;
        if (isDisused[i] == 0)
            phys_nodes[nodekey].set_disused(false);
        else
            phys_nodes[nodekey].set_disused(true);
        auto keywords = split_keyword_list(KeywordsList[i]);
        for (auto keyword : keywords)
        {
            phys_nodes[nodekey].add_keyword(keyword);
        }
        std::vector<float> pos(2);
        pos[0] = abs_pos[i * 2];
        pos[1] = abs_pos[i * 2 + 1];
        phys_nodes[nodekey].set_position(pos);
        phys_nodes[nodekey].set_new(false);
        phys_nodes[nodekey].set_group_index(i);
        // node_name_2_node.try_emplace(phys_nodes[nodekey].get_complete_name_spec(),
        // &phys_nodes[nodekey]);
        name2_phys_node.emplace(phys_nodes[nodekey].get_complete_name_spec(), &phys_nodes[nodekey]);
    }
}

// private method
void wanda_model::read_signal_lines()
{
    num_signal_lines = 0;
    int numrecords = wanda_input_file.get_maxdim_index("SIGNAL_LINES");

    if (numrecords <= 0)
    {
        return;
    }
    const nefis_uindex uindex = {1, numrecords, 1};
    std::vector<std::string> Name(numrecords);
    wanda_input_file.get_string_element("SIGNAL_LINES", "Name", uindex, 0, Name);
    std::vector<std::string> sig_line_key(numrecords);
    wanda_input_file.get_string_element("SIGNAL_LINES", "Sig_line_key", uindex, 8, sig_line_key);
    std::vector<int> isDisused(numrecords);
    wanda_input_file.get_int_element("SIGNAL_LINES", "Is_disused", {1, numrecords, 1}, isDisused);
    std::vector<std::string> KeywordsList(numrecords);
    wanda_input_file.get_string_element("SIGNAL_LINES", "Keywords", uindex, 50, KeywordsList);
    std::vector<std::string> comment(numrecords);
    wanda_input_file.get_string_element("SIGNAL_LINES", "Comment", uindex, 50, comment);
    std::vector<std::string> user_name(numrecords);
    wanda_input_file.get_string_element("SIGNAL_LINES", "User_name", uindex, 24, user_name);
    std::vector<std::string> date_mod(numrecords);
    wanda_input_file.get_string_element("SIGNAL_LINES", "Date_time_modify", uindex, 17, date_mod);
    std::vector<std::string> signal_type(numrecords);
    wanda_input_file.get_string_element("SIGNAL_LINES", "Signal_type", uindex, 8, signal_type);
    for (int i = 0; i < numrecords; i++)
    {
        auto sig_key = sig_line_key[i];
        if (sig_key.find("Free") != sig_key.npos)
            continue;

        int key = strtol(sig_key.substr(1).c_str(), nullptr, 10);
        wanda_sig_line sig_line(key, Name[i]);
        sig_line.set_group_index(i + 1);
        num_signal_lines++;

        if (isDisused[i] == 0)
            sig_line.set_disused(false);
        else
            sig_line.set_disused(true);

        auto keywords = split_keyword_list(KeywordsList[i]);
        for (auto keyword : keywords)
        {
            sig_line.add_keyword(keyword);
        }
        sig_line.set_comment(comment[i]);
        sig_line.set_date_mod(date_mod[i]);
        sig_line.set_user_name(user_name[i]);
        sig_line.set_signal_line_type(signal_type[i]);
        auto name = sig_line.get_complete_name_spec();
        std::vector<std::vector<std::string>> C_comp_keys(1, std::vector<std::string>(2));
        wanda_input_file.get_string_element("SIGNAL_LINES", "C_comp_keys", {i + 1, i + 1, 1}, {1, 2, 1}, 8,
                                            C_comp_keys);
        std::vector<int> chn_index(2);
        wanda_input_file.get_int_element("SIGNAL_LINES", "Sig_chnl_ndx", {i + 1, i + 1, 1}, chn_index);
        sig_line.set_con_point(chn_index);
        signal_lines.emplace(sig_key, sig_line);
        // connecting the signal line to components
        for (int j = 0; j <= 1; j++)
        {
            wanda_component *comp = nullptr;

            if (ctrl_components.find(C_comp_keys[0][j]) != ctrl_components.end())
            {
                comp = &ctrl_components[C_comp_keys[0][j]];
            }
            else if (phys_components.find(C_comp_keys[0][j]) != phys_components.end())
            {
                comp = &phys_components[C_comp_keys[0][j]];
            }
            else
            {
                _model_is_corrupt = true;
                delete_sig_line(signal_lines[sig_key]);
                continue;
            }
            try
            {
                if (j == 0)
                {
                    signal_lines[sig_key].connect_output(*comp, chn_index[j]);
                }
                else
                {
                    signal_lines[sig_key].connect_input(*comp, chn_index[j]);
                }
            }
            catch (std::invalid_argument &error)
            {
                _model_is_corrupt = true;
                delete_sig_line(signal_lines[sig_key]);
                continue;
            }
            comp->connect(signal_lines[sig_key], chn_index[j], j == 1);
        }

        // sig_line_name_2_sig_line.emplace(name ,&signal_lines[sig_key]);
        sig_line_keys.push_back(sig_key);
        signal_lines[sig_key].set_new(false);
    }
}

// private method
void wanda_model::read_physical_comp()
{
    phys_components.clear();
    number_physical_components = 0;
    int numrecords = wanda_input_file.get_maxdim_index("H_COMPONENTS");

    if (numrecords <= 0)
    {
        return;
        // std::string message = "No Physical components found in this Wanda case: "
        // + wanda_input_file.get_filename();
        // throw(message);
    }
    nefis_uindex uindex = {1, numrecords, 1};
    std::vector<std::string> Class_name(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Class_name", uindex, 8, Class_name);

    std::vector<std::string> Class_sort_key(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Class_sort_key", uindex, 8, Class_sort_key);

    std::vector<std::string> Name(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Name", uindex, 0, Name);

    std::vector<std::string> type(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Comp_type", uindex, 8, type);

    // std::vector<std::string> sort_name(numrecords);
    // wanda_input_file.get_string_element("H_COMPONENTS", "Sort_name", 1,
    // numrecords, 1, 12, sort_name);

    std::vector<std::string> H_comp_key(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "H_comp_key", uindex, 8, H_comp_key);

    std::vector<std::string> KeywordsList(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Keywords", uindex, 50, KeywordsList);

    std::vector<int> isDisused(numrecords);
    wanda_input_file.get_int_element("H_COMPONENTS", "Is_disused", {1, numrecords, 1}, isDisused);

    std::vector<int> N_elements(numrecords);
    wanda_input_file.get_int_element("H_COMPONENTS", "N_elements", {1, numrecords, 1}, N_elements);

    //  std::vector<int> use_action_table(numrecords);
    //  wanda_input_file.get_int_element("H_COMPONENTS", "Use_action_table", 1,
    //  numrecords, 1, use_action_table);

    // std::vector<std::string> action_table_key(numrecords);
    // wanda_input_file.get_string_element("H_COMPONENTS", "Action_table_key", 1,
    // numrecords, 1, 8, action_table_key);

    std::vector<float> centre_pos(numrecords * 2);
    wanda_input_file.get_float_element("H_COMPONENTS", "Comp_centre_pos", {1, numrecords, 1}, centre_pos);

    std::vector<std::string> spec_oper_key(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Spec_oper_key", uindex, 8, spec_oper_key);

    int numrecords_comspec = wanda_input_file.get_maxdim_index("H_COM_SPEC_VAL");

    std::vector<std::string> spec_com_key(numrecords_comspec);
    wanda_input_file.get_string_element("H_COM_SPEC_VAL", "Spec_comm_key", {1, numrecords_comspec, 1}, 8, spec_com_key);

    int numrecords3 = wanda_input_file.get_maxdim_index("H_OPE_SPEC_VAL");
    std::vector<std::string> spec_com_key_ope(numrecords3);
    wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_comm_key", {1, numrecords3, 1}, 8, spec_com_key_ope);
    std::vector<std::string> spec_oper_key_ope(numrecords3);
    wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_oper_key", {1, numrecords3, 1}, 8, spec_oper_key_ope);
    std::vector<std::string> comment(numrecords);

    // loop through all components in the case, and add them to our list
    for (auto i = 0; i <= numrecords - 1; i++)
    {
        auto compkey = H_comp_key[i];
        if (compkey.find("Free") != compkey.npos) // If there was a component at this record, but has been
                                                  // removed....
            continue;
        if (Class_sort_key[i] == "TNK1_MS2")
        {
            num_of_species += 1;
            if (num_of_species > max_num_of_species)
            {
                throw std::runtime_error("Number of different species is greater than the maximum which is " +
                                         std::to_string(num_of_species));
            }
        }
        auto key = strtol(compkey.substr(1).c_str(), nullptr, 10);
        wanda_component *comp_point = component_definition->get_component(Class_sort_key[i]);
        phys_components.emplace(compkey, *comp_point);
        phys_components[compkey].set_physcomp_type(type[i]);
        phys_components[compkey].set_name(Name[i]);
        phys_components[compkey].set_num_elements(N_elements[i]);
        phys_components[compkey].set_comp_key(key);
        phys_components[compkey].set_class_name(Class_name[i]);
        phys_components[compkey].set_group_index(i);
        if (isDisused[i] == 0)
        {
            phys_components[compkey].set_disused(false);
        }
        else
        {
            phys_components[compkey].set_disused(true);
        }
        auto keywords = split_keyword_list(KeywordsList[i]);
        for (auto keyword : keywords)
        {
            phys_components[compkey].add_keyword(keyword);
        }
        std::vector<float> pos(2);
        pos[0] = centre_pos[2 * i];
        pos[1] = centre_pos[2 * i + 1];
        phys_components[compkey].set_position(pos);
        phys_components[compkey].set_comp_num(i + 1);
        phys_components[compkey].set_new(false);
        number_physical_components++;
        name2_phys_comp.emplace(phys_components[compkey].get_complete_name_spec(), &phys_components[compkey]);
        // comp_name_2_comp.emplace(phys_components[compkey].get_complete_name_spec(),
        // &phys_components[compkey]);
        if (phys_components[compkey].contains_property("Composition 1 1"))
        {
            species_stride += phys_components[compkey].get_number_of_connnect_points() - 1 +
                              phys_components[compkey].get_num_elements();
        }
    }
    number_physical_components = number_physical_components;
}

// private method
void wanda_model::read_control_comp()
{
    number_control_components = 0;
    int numrecords = wanda_input_file.get_maxdim_index("C_COMPONENTS");

    if (numrecords <= 0)
    {
        std::string message = "No control components found in this Wanda case: " + wanda_input_file.get_filename();
        return;
    }
    const nefis_uindex c_comp_uindex = {1, numrecords, 1};
    std::vector<std::string> Class_name(numrecords);
    wanda_input_file.get_string_element("C_COMPONENTS", "Class_name", c_comp_uindex, 8, Class_name);

    std::vector<std::string> Name(numrecords);
    wanda_input_file.get_string_element("C_COMPONENTS", "Name", c_comp_uindex, 0, Name);

    std::vector<std::string> C_comp_key(numrecords);
    wanda_input_file.get_string_element("C_COMPONENTS", "C_comp_key", c_comp_uindex, 8, C_comp_key);

    std::vector<std::string> KeywordsList(numrecords);
    wanda_input_file.get_string_element("C_COMPONENTS", "Keywords", c_comp_uindex, 50, KeywordsList);

    std::vector<int> isDisused(numrecords);
    wanda_input_file.get_int_element("C_COMPONENTS", "Is_disused", {1, numrecords, 1}, isDisused);

    std::vector<float> centrpos(2 * numrecords);
    wanda_input_file.get_float_element("C_COMPONENTS", "Comp_touch_pos", {1, numrecords, 1}, centrpos);

    // loop through all components in the case, and add them to our list
    for (int i = 0; i <= numrecords - 1; i++)
    {
        auto compkey = C_comp_key[i];
        if (compkey.find("Free") != compkey.npos)
            continue;
        auto key = strtol(compkey.substr(1).c_str(), nullptr, 10);
        std::string Name_prefix = component_definition->get_name_prefix_ctrl_comp(Class_name[i]);
        ctrl_components.emplace(compkey, *component_definition->get_component(Class_name[i]));
        // wanda_component comp(key, Class_name[i], Class_name[i], Name_prefix,
        // Name[i], control, "CTRL");
        ctrl_components[compkey].set_comp_num(i);
        if (isDisused[i] == 0)
            ctrl_components[compkey].set_disused(false);
        else
            ctrl_components[compkey].set_disused(true);

        auto keywords = split_keyword_list(KeywordsList[i]);
        for (auto keyword : keywords)
        {
            ctrl_components[compkey].add_keyword(keyword);
        }
        ctrl_components[compkey].set_max_input_channels(component_definition->get_max_in_chan(Class_name[i]));
        ctrl_components[compkey].set_min_input_channels(component_definition->get_min_in_chan(Class_name[i]));
        ctrl_components[compkey].set_input_channel_type(component_definition->get_in_chan_type(Class_name[i]));
        ctrl_components[compkey].set_output_channel_type(component_definition->get_out_chan_type(Class_name[i]));
        ctrl_components[compkey].set_name(Name[i]);
        ctrl_components[compkey].set_comp_key(key);
        // ctrl_components.emplace(compkey, ctrl_components[compkey]);
        ctrl_components[compkey].set_new(false);
        ctrl_components[compkey].set_group_index(i);
        std::vector<float> pos(2);
        pos[0] = centrpos[2 * i];
        pos[1] = centrpos[2 * i + 1];
        ctrl_components[compkey].set_position(pos);
        // comp_name_2_comp.emplace(comp.get_complete_name_spec(),
        // &ctrl_components[compkey]);
        number_control_components++;
    }
}

// private method
void wanda_model::read_phys_component_input()
{
    if (number_physical_components <= 0)
    {
        return;
    }

    int numrecords = wanda_input_file.get_maxdim_index("H_COMPONENTS");
    const nefis_uindex h_comp_uindex = {1, numrecords, 1};

    std::vector<std::string> H_comp_key(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "H_comp_key", h_comp_uindex, 8, H_comp_key);

    std::vector<std::string> spec_oper_key(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Spec_oper_key", h_comp_uindex, 8, spec_oper_key);

    std::vector<std::string> action_table_key(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Org_act_tbl_key", h_comp_uindex, 8, action_table_key);
    std::vector<int> use_action_table(numrecords);
    wanda_input_file.get_int_element("H_COMPONENTS", "Use_action_table", {1, numrecords, 1}, use_action_table);

    int numrecords_comspec = wanda_input_file.get_maxdim_index("H_COM_SPEC_VAL");
    std::vector<int> N_his_com(numrecords_comspec);
    wanda_input_file.get_int_element("H_COM_SPEC_VAL", "N_his", {1, numrecords_comspec, 1}, N_his_com);
    std::vector<std::string> spec_com_key(numrecords_comspec);
    wanda_input_file.get_string_element("H_COM_SPEC_VAL", "Spec_comm_key", {1, numrecords_comspec, 1}, 8, spec_com_key);

    int numrecords_operspec = wanda_input_file.get_maxdim_index("H_OPE_SPEC_VAL");
    std::vector<int> N_his_ope(numrecords_operspec);
    wanda_input_file.get_int_element("H_OPE_SPEC_VAL", "N_his", {1, numrecords_operspec, 1}, N_his_ope);
    std::vector<int> N_hcs(numrecords_operspec);
    wanda_input_file.get_int_element("H_OPE_SPEC_VAL", "N_hcs", {1, numrecords_operspec, 1}, N_hcs);

    std::vector<std::string> spec_com_key_ope(numrecords_operspec);
    wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_comm_key", {1, numrecords_operspec, 1}, 8,
                                        spec_com_key_ope);
    std::vector<std::string> spec_oper_key_ope(numrecords_operspec);
    wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_oper_key", {1, numrecords_operspec, 1}, 8,
                                        spec_oper_key_ope);
    std::vector<std::string> comment(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Comment", h_comp_uindex, 50, comment);
    std::vector<std::string> user_name(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "User_name", h_comp_uindex, 24, user_name);
    std::vector<std::string> date_mod(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Date_time_modify", h_comp_uindex, 17, date_mod);
    std::vector<std::string> mode_name(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Model_name", h_comp_uindex, 24, mode_name);
    std::vector<std::string> ref_id(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Reference_id", h_comp_uindex, 120, ref_id);
    std::vector<std::string> mat_name(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Material_name", h_comp_uindex, 24, mat_name);
    std::vector<int> seq_num(numrecords);
    wanda_input_file.get_int_element("H_COMPONENTS", "Sort_sequence", {1, numrecords, 1}, seq_num);
    std::vector<float> angle(numrecords);
    wanda_input_file.get_float_element("H_COMPONENTS", "Rotate_angle", {1, numrecords, 1}, angle);

    // Flip_vertical is also available in wdi-file, but isn't used for Wanda.
    // Flip_horizontal is used for both horizontal and vertical flips. For
    // vertical flips, the rotation angle is set to account for the vertical flip.
    std::vector<int> flip_horizontal(numrecords);
    wanda_input_file.get_int_element("H_COMPONENTS", "Flip_horizontal", {1, numrecords, 1}, flip_horizontal);

    std::unordered_map<std::string, int> H_comp_keys;
    std::unordered_map<std::string, int> spec_oper_key_opes;
    std::unordered_map<std::string, int> spec_com_keys;
    int loop_size = max(max(H_comp_key.size(), spec_oper_key_ope.size()), spec_com_key.size());

    for (int i = 0; i < loop_size; i++)
    {
        if (i < H_comp_key.size())
        {
            if (H_comp_key[i] != "Free")
            {
                H_comp_keys[H_comp_key[i]] = i;
            }
        }
        if (i < spec_oper_key_ope.size())
        {
            if (spec_oper_key_ope[i] != "Free")
            {
                spec_oper_key_opes[spec_oper_key_ope[i]] = i;
            }
        }
        if (i < spec_com_key.size())
        {
            if (spec_com_key[i] != "Free")
            {
                spec_com_keys[spec_com_key[i]] = i;
            }
        }
    }
    std::vector<float> global_hsc = get_globvar_hcs();
    for (auto &item : phys_components)
    {
        auto &comp = item.second;
        comp.set_number_of_species(&num_of_species);
        auto name = comp.get_complete_name_spec();
        int index = H_comp_keys[comp.get_key_as_string()] + 1;
        // get_key_index_array( H_comp_key, comp.get_key_as_string()) + 1;  //Add
        // one to adjust for NEFIS 1-based arrays int index = comp.comp_num + 1;
        // setting overall props
        if (index == 0)
            throw std::invalid_argument("Component doesn't exist in case: " + comp.get_complete_name_spec() + " - " +
                                        comp.get_key_as_string());

        comp.set_comment(comment[index - 1]);
        comp.set_date_mod(date_mod[index - 1]);
        if (comp.is_pipe())
        {
            comp.set_material_name(mat_name[index - 1]);
        }
        comp.set_model_name(mode_name[index - 1]);
        comp.set_ref_id(ref_id[index - 1]);
        comp.set_sequence_number(seq_num[index - 1]);
        comp.set_user_name(user_name[index - 1]);
        comp.set_angle(angle[index - 1]);
        comp.set_flipped(flip_horizontal[index - 1] != 0);

        int index_ope = min(index, spec_oper_key_ope.size());

        if (spec_oper_key_ope[index_ope - 1].compare(spec_oper_key[index - 1]) != 0)
        {
            index_ope = spec_oper_key_opes[spec_oper_key[index - 1]] + 1;
            // get_key_index_array(spec_oper_key_ope, spec_oper_key[index - 1]) + 1;
        }
        comp.set_oper_index(index_ope);
        int index_com = min(index_ope, spec_com_key.size());
        if (spec_com_key[index_com - 1].compare(spec_com_key_ope[index_com - 1]) != 0)
        {
            index_com = spec_com_keys[spec_com_key_ope[index_ope - 1]] + 1;
            // get_key_index_array(spec_com_key, spec_com_key_ope[index_ope - 1]) + 1;
        }
        comp.set_com_index(index_com);
        // input spec
        std::vector<float> his_com_numval(36);
        wanda_input_file.get_float_element("H_COM_SPEC_VAL", "Spec_numval_his", {index_com, index_com, 1},
                                           his_com_numval);
        std::vector<std::string> com_spec_status_his(36);
        wanda_input_file.get_string_element("H_COM_SPEC_VAL", "Spec_status_his", {index_com, index_com, 1}, 1,
                                            com_spec_status_his);

        std::vector<std::string> his_com_chrval(36);
        wanda_input_file.get_string_element("H_COM_SPEC_VAL", "Spec_chrval_his", {index_com, index_com, 1}, 16,
                                            his_com_chrval);
        // operation specs
        std::vector<float> his_ope_numval(36);
        wanda_input_file.get_float_element("H_OPE_SPEC_VAL", "Spec_numval_his", {index_ope, index_ope, 1},
                                           his_ope_numval);
        std::vector<std::string> ope_spec_status_his(36);
        wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_status_his", {index_ope, index_ope, 1}, 1,
                                            ope_spec_status_his);
        std::vector<std::string> his_ope_chrval(36);
        wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_chrval_his", {index_ope, index_ope, 1}, 16,
                                            his_ope_chrval);

        std::vector<std::string> con_hnodes(4);
        wanda_input_file.get_string_element("H_COMPONENTS", "H_node_keys", {index, index, 1}, 8, con_hnodes);

        for (int i = 0; i < 4; i++)
        {
            if (con_hnodes[i] != unref)
            {
                // int key = strtol(con_hnodes[i].substr(1).c_str(), nullptr, 10);
                auto &node = phys_nodes[con_hnodes[i]];
                comp.connect(node, i + 1);
                node.connect(comp);
            }
        }

        for (auto &componentProperty : comp)
        {
            if (componentProperty.second.get_property_type() != wanda_property_types::HIS)
                continue;

            componentProperty.second.set_group_index(index);
            if (componentProperty.second.get_property_spec_inp_fld() == 'C' ||
                componentProperty.second.get_property_spec_inp_fld() == 'R' ||
                componentProperty.second.get_property_spec_inp_fld() == 'I')
            { // Choice (char), float or integer properties
                if (componentProperty.second.get_property_spec_code() == 'C')
                { // Common specs
                    componentProperty.second.set_spec_status(
                        com_spec_status_his[componentProperty.second.get_index()] == "H");
                    if (componentProperty.second.get_spec_status())
                    {
                        componentProperty.second.set_scalar(his_com_numval[componentProperty.second.get_index()]);
                    }
                }
                else if (componentProperty.second.get_property_spec_code() == 'O')
                {
                    componentProperty.second.set_spec_status(
                        ope_spec_status_his[componentProperty.second.get_index()] == "H");
                    if (componentProperty.second.get_spec_status())
                    {
                        componentProperty.second.set_scalar(his_ope_numval[componentProperty.second.get_index()]);
                    }
                }
                else
                {
                    // Error!
                }
                componentProperty.second.set_modified(false);
            }
            else
            {
                // table, num col or string col.
                wanda_table &table = componentProperty.second.get_table();
                std::vector<std::string> description = table.get_descriptions();
                for (auto tab_description : table.get_descriptions())
                {
                    if (componentProperty.first == "Action table")
                    {
                        table.set_key(tab_description, action_table_key[index - 1]);
                        comp.set_use_action_table(use_action_table[index - 1] == 1);
                    }
                    else if (componentProperty.second.get_table().get_spec_code(tab_description) == 'C')
                    {
                        table.set_key(tab_description, his_com_chrval[table.get_index(tab_description)]);
                    }
                    else
                    {
                        table.set_key(tab_description, his_ope_chrval[table.get_index(tab_description)]);
                    }
                }
                read_table(table);
            }
        }

        if (comp.is_pipe() && !comp.is_disused())
        {            
            calc_hsc(comp);
            int nel = comp.get_property("Pipe element count").get_scalar_float();
            if (nel == 0)
            {
                std::vector<int> vec_nel(1);
                wanda_input_file.get_int_element("H_COMPONENTS", "N_elements", {index, index, 1}, vec_nel);
                nel = vec_nel[0];
            }
            comp.set_num_elements(nel);
        }
    }
}

// private method
void wanda_model::read_phys_node_input()
{
    if (number_physical_nodes <= 0)
    {
        return;
    }

    auto numrecords = wanda_input_file.get_maxdim_index("H_NODES");
    const nefis_uindex h_nodes_uindex = {1, numrecords, 1};
    std::vector<std::string> sbH_node_key(numrecords);
    wanda_input_file.get_string_element("H_NODES", "H_node_key", h_nodes_uindex, 8, sbH_node_key);
    std::vector<int> N_nis(numrecords);
    wanda_input_file.get_int_element("H_NODES", "N_nis", {1, numrecords, 1}, N_nis);
    std::vector<std::string> comment(numrecords);
    wanda_input_file.get_string_element("H_NODES", "Comment", h_nodes_uindex, 50, comment);
    std::vector<std::string> user_name(numrecords);
    wanda_input_file.get_string_element("H_NODES", "User_name", h_nodes_uindex, 24, user_name);
    std::vector<std::string> date_mod(numrecords);
    wanda_input_file.get_string_element("H_NODES", "Date_time_modify", h_nodes_uindex, 17, date_mod);
    std::vector<int> seq_num(numrecords);
    wanda_input_file.get_int_element("H_NODES", "Sort_sequence", {1, numrecords, 1}, seq_num);

    std::unordered_map<std::string, int> sbH_node_keys;
    for (int i = 0; i < sbH_node_key.size(); i++)
    {
        sbH_node_keys[sbH_node_key[i]] = i;
    }
    for (auto &item : phys_nodes)
    {
        auto &node = item.second;
        auto index = sbH_node_keys[node.get_key_as_string()] + 1;
        // get_key_index_array( sbH_node_key, node.get_key_as_string()) + 1;  //Add
        // one to adjust for NEFIS 1-based arrays

        node.set_comment(comment[index - 1]);
        node.set_date_mod(date_mod[index - 1]);
        node.set_sequence_number(seq_num[index - 1]);
        node.set_user_name(user_name[index - 1]);
        std::vector<float> Nis_numval(36);
        wanda_input_file.get_float_element("H_NODES", "Spec_numval_nis", {index, index, 1}, Nis_numval);
        std::vector<std::string> spec_status_nis(36);
        wanda_input_file.get_string_element("H_NODES", "Spec_status_nis", {index, index, 1}, 1, spec_status_nis);
        std::vector<std::string> Nis_chrval(36);
        wanda_input_file.get_string_element("H_NODES", "Spec_chrval_nis", {index, index, 1}, 16, Nis_chrval);
        for (auto &inputproperty : node)
        {
            if (inputproperty.second.get_property_type() != wanda_property_types::NIS)
                continue;
            inputproperty.second.set_group_index(index);

            if (inputproperty.second.get_property_spec_inp_fld() == 'C' ||
                inputproperty.second.get_property_spec_inp_fld() == 'R' ||
                inputproperty.second.get_property_spec_inp_fld() == 'I')
            {
                // Choice (char), float or integer properties
                inputproperty.second.set_spec_status(spec_status_nis[inputproperty.second.get_index()] == "H");
                if (inputproperty.second.get_spec_status())
                {
                    inputproperty.second.set_scalar(Nis_numval[inputproperty.second.get_index()]);
                }
            }
            else
            {
                // table, num col or string col.
                wanda_table &table = inputproperty.second.get_table();
                std::vector<std::string> description = table.get_descriptions();
                for (auto tab_description : table.get_descriptions())
                {
                    table.set_key(tab_description, Nis_chrval[table.get_index(tab_description)]);
                }
                read_table(table);
            }
        }
    }
}

// private method
void wanda_model::read_ctrl_component_input()
{
    if (number_control_components <= 0)
    {
        return;
    }
    auto numrecords = wanda_input_file.get_maxdim_index("C_COMPONENTS");
    const nefis_uindex c_comp_uindex = {1, numrecords, 1};
    std::vector<std::string> sbC_comp_key(numrecords);
    wanda_input_file.get_string_element("C_COMPONENTS", "C_comp_key", c_comp_uindex, 8, sbC_comp_key);
    std::vector<int> N_cis(numrecords);
    wanda_input_file.get_int_element("C_COMPONENTS", "N_cis", {1, numrecords, 1}, N_cis);
    std::vector<std::string> comment(numrecords);
    wanda_input_file.get_string_element("C_COMPONENTS", "Comment", c_comp_uindex, 50, comment);
    std::vector<std::string> user_name(numrecords);
    wanda_input_file.get_string_element("C_COMPONENTS", "User_name", c_comp_uindex, 24, user_name);
    std::vector<std::string> date_mod(numrecords);
    wanda_input_file.get_string_element("C_COMPONENTS", "Date_time_modify", c_comp_uindex, 17, date_mod);
    std::vector<std::string> ref_id(numrecords);
    wanda_input_file.get_string_element("C_COMPONENTS", "Reference_id", c_comp_uindex, 120, ref_id);
    std::vector<int> seq_num(numrecords);
    wanda_input_file.get_int_element("C_COMPONENTS", "Sort_sequence", {1, numrecords, 1}, seq_num);

    // loading signal line info
    std::vector<std::string> sig_line_keys(num_signal_lines);
    std::vector<std::vector<std::string>> sig_c_comp(num_signal_lines, std::vector<std::string>(2));
    if (num_signal_lines > 0)
    {
        wanda_input_file.get_string_element("SIGNAL_LINES", "Sig_line_key", {1, num_signal_lines, 1}, 8, sig_line_keys);

        wanda_input_file.get_string_element("SIGNAL_LINES", "C_comp_keys", {1, num_signal_lines, 1}, {1, 2, 1}, 8,
                                            sig_c_comp);
    }

    for (auto &wanda_comp : ctrl_components)
    {
        // auto index = get_key_index_array( sbC_comp_key,
        // wandaComponent.second.get_key_as_string()) + 1; //+1 to adjust for NEFIS
        // 1-based arrays
        int index = wanda_comp.second.get_comp_num() + 1;
        if (index == 0)
            throw std::invalid_argument("Component doesn't exist in case: " + wanda_comp.second.get_key_as_string());

        wanda_comp.second.set_comment(comment[index - 1]);
        wanda_comp.second.set_date_mod(date_mod[index - 1]);
        wanda_comp.second.set_ref_id(ref_id[index - 1]);
        wanda_comp.second.set_sequence_number(seq_num[index - 1]);
        wanda_comp.second.set_user_name(user_name[index - 1]);

        std::vector<float> Cis_numval(36);
        wanda_input_file.get_float_element("C_COMPONENTS", "Spec_numval_cis", {index, index, 1}, Cis_numval);
        std::vector<std::string> spec_status_cis(36);
        wanda_input_file.get_string_element("C_COMPONENTS", "Spec_status_cis", {index, index, 1}, 1, spec_status_cis);
        std::vector<std::string> Cis_chrval(36);
        wanda_input_file.get_string_element("C_COMPONENTS", "Spec_chrval_cis", {index, index, 1}, 16, Cis_chrval);
        std::vector<std::string> h_node_key(1);
        wanda_input_file.get_string_element("C_COMPONENTS", "H_comp_key", {index, index, 1}, 8, h_node_key);
        std::vector<int> h_comp_con(1);
        wanda_input_file.get_int_element("C_COMPONENTS", "C_comp_hcomp_con", {index, index, 1}, h_comp_con);

        // setting the connections of the component.
        // TODO fix setting the proper connection

        // why only check the node and not the components?
        if (phys_nodes.find(h_node_key[0]) != phys_nodes.end())
        {
            auto &node = phys_nodes[h_node_key[0]];
            // wanda_comp.second.connect(node, 1);
            connect(wanda_comp.second, 1, node);
        }

        // loading input data of the components
        for (auto &property : wanda_comp.second)
        {
            if (property.second.get_property_type() != wanda_property_types::CIS)
                continue;
            property.second.set_group_index(index);

            if (property.second.get_property_spec_inp_fld() == 'C' ||
                property.second.get_property_spec_inp_fld() == 'R' ||
                property.second.get_property_spec_inp_fld() == 'I')
            { // Choice (char), float or integer properties
                property.second.set_spec_status(spec_status_cis[property.second.get_index()] == "H");
                if (property.second.get_spec_status())
                {
                    property.second.set_scalar(Cis_numval[property.second.get_index()]);
                }
            }
            else
            {
                // table, num col or string col.
                wanda_table &table = property.second.get_table();
                std::vector<std::string> description = table.get_descriptions();
                for (auto tab_description : table.get_descriptions())
                {
                    table.set_key(tab_description, Cis_chrval[table.get_index(tab_description)]);
                }
                read_table(table);
            }
        }
        if (wanda_comp.second.get_class_sort_key() == "SENSOR")
        {
            if (wanda_comp.second.is_node_connected(1))
            {
                wanda_comp.second.fill_sensor_list(wanda_comp.second.get_connected_node(1));
            }
            else if (wanda_comp.second.is_sigline_connected(1, true))
            {
                auto sigline = wanda_comp.second.get_connected_sigline(1, true);
                wanda_component *temp_comp = sigline[0]->get_output_component();
                int con_point = temp_comp->get_connect_point(*sigline[0]);
                wanda_comp.second.fill_sensor_list(*temp_comp, con_point);
            }
        }
    }
}

template <typename T>
std::string wanda_model::get_unique_key(std::unordered_map<std::string, T> *keys, char prefix, int &i)
{
    std::string table_key = prefix + int2string(i, 7);
    // ReSharper disable once CppInitializedValueIsAlwaysRewritten    
    do
    {
        i++;
        table_key = prefix + int2string(i, 7);
    } while (keys->contains(table_key));
    return table_key;
}


std::string wanda_model::get_unique_key(std::vector<std::string> keys, char prefix, int &i)
{
    std::string table_key = prefix + int2string(i, 7);
    // ReSharper disable once CppInitializedValueIsAlwaysRewritten
    do
    {
        i++;
        table_key = prefix + int2string(i, 7);
    } while (get_key_index_array(keys, (table_key)) + 1 != 0);
    return table_key;
}


// private method
void wanda_model::reload_component_indices()
{
    output_index_group_cache.clear(); // clear cache
#ifdef DEBUG
    std::cout << "Reloading component indices\n";
#endif
    auto N_group = wanda_output_file.get_maxdim_index("H_COMP_INDEX");
    std::vector<std::string> PhysCompKeys(N_group);
    wanda_output_file.get_string_element("H_COMP_INDEX", "H_comp_key", {1, N_group, 1}, 8, PhysCompKeys);
    std::unordered_map<std::string, int> h_comp_index;
    for (int i = 0; i < N_group; i++)
    {
        h_comp_index.emplace(PhysCompKeys[i], i);
    }

    for (auto &comp : phys_components)
    {
#ifdef DEBUG
        std::cout << "Reloading component indices for " << comp.second.get_complete_name_spec() << '\n';
#endif
        if (!comp.second.is_disused())
        {
            for (auto &item : comp.second)
            {
                if (item.second.get_property_type() == wanda_property_types::GLOQUANT ||
                    item.second.get_property_type() == wanda_property_types::HOS ||
                    item.second.get_property_type() == wanda_property_types::HOV)
                {
                    std::vector<int> OutputIndices(N_group);
                    if (output_index_group_cache.find(item.second.get_wdo_postfix()) == output_index_group_cache.end())
                    {
                        std::string ElementName = "Ndx_";
                        ElementName += to_lower(item.second.get_wdo_postfix());
                        wanda_output_file.get_int_element("H_COMP_INDEX", ElementName, {1, N_group, 1}, OutputIndices);
                        output_index_group_cache[item.second.get_wdo_postfix()] = OutputIndices;
                    }
                    else
                    {
                        OutputIndices = output_index_group_cache[item.second.get_wdo_postfix()];
                    }

                    int KeyIndex = h_comp_index[comp.second.get_key_as_string()];
                    item.second.set_group_index(OutputIndices[KeyIndex]);
                    // for glo quants you need to determine which gloquant it is. and if it
                    // is a pipe you need include the number of elements.
                    // if it is a composition you need to take that into account
                    if (item.second.is_glo_quant())
                    {
                        if (item.second.get_species_number() <= num_of_species)
                        {
                            // std::string temp =
                            //   (item.second.get_description())
                            //   .substr((item.second.get_description()).size() - 1, 1);
                            // int con = strtol(temp.c_str(), nullptr, 10);
                            int con = item.second.get_connection_point() + 1;

                            int nel = comp.second.get_num_elements();
                            int group_index = item.second.get_group_index();
                            // needs to be corrected with the stride
                            if (nel != 0)
                            {
                                auto tempval = item.second.get_species_number() == 0
                                                   ? 0
                                                   : (item.second.get_species_number() - 1) * species_stride;
                                group_index += ((con - 1) * (nel) + tempval);
                            }
                            else
                            {
                                auto tempval = item.second.get_species_number() == 0
                                                   ? 0
                                                   : (item.second.get_species_number() - 1) * species_stride;
                                group_index += (con - 1) + tempval;
                            }
                            item.second.set_group_index(group_index);
                        }
                    }
                }
            }
        }
    }
    N_group = wanda_output_file.get_maxdim_index("H_NODE_INDEX");
    std::vector<std::string> PhysNodeKeys(N_group);
    wanda_output_file.get_string_element("H_NODE_INDEX", "H_node_key", {1, N_group, 1}, 8, PhysNodeKeys);

    std::unordered_map<std::string, int> h_node_index;
    for (int i = 0; i < N_group; i++)
    {
        h_node_index.emplace(PhysNodeKeys[i], i);
    }

    output_index_group_cache.clear(); // clear cache
    for (auto &node : phys_nodes)
    {
#ifdef DEBUG
        std::cout << "Reloading component indices for " << node.second.get_complete_name_spec() << '\n';
#endif
        if (!node.second.is_disused())
        {
            for (auto &item : node.second)
            {
                if (item.second.get_property_type() == wanda_property_types::GLOQUANT ||
                    item.second.get_property_type() == wanda_property_types::NOV ||
                    item.second.get_property_type() == wanda_property_types::NOS)
                {
                    std::vector<int> OutputIndices(N_group);

                    if (output_index_group_cache.find(item.second.get_wdo_postfix()) == output_index_group_cache.end())
                    { // this index hasn't been found yet.
                        std::string ElementName = "Ndx_";
                        ElementName += to_lower(item.second.get_wdo_postfix());
                        wanda_output_file.get_int_element("H_NODE_INDEX", ElementName, {1, N_group, 1}, OutputIndices);
                        output_index_group_cache[item.second.get_wdo_postfix()] = OutputIndices;
                    }
                    else
                    {
                        OutputIndices = output_index_group_cache[item.second.get_wdo_postfix()];
                    }
                    int KeyIndex = h_node_index[node.second.get_key_as_string()];
                    item.second.set_group_index(OutputIndices[KeyIndex]);
                }
            }
        }
    }
    output_index_group_cache.clear(); // clear cache
    N_group = wanda_output_file.get_maxdim_index("C_COMP_INDEX");
    if (N_group >= 1)
    {
        std::vector<std::string> CtrlCompKeys(N_group);
        wanda_output_file.get_string_element("C_COMP_INDEX", "C_comp_key", {1, N_group, 1}, 8, CtrlCompKeys);
        std::unordered_map<std::string, int> ctrl__index;
        for (int i = 0; i < N_group; i++)
        {
            ctrl__index.emplace(CtrlCompKeys[i], i);
        }

        for (auto &comp : ctrl_components)
        {
#ifdef DEBUG
            std::cout << "Reloading component indices for " << comp.second.get_complete_name_spec() << '\n';
#endif
            if (!comp.second.is_disused())
            {
                for (auto &item : comp.second)
                {
                    if (item.second.get_property_type() == wanda_property_types::GLOQUANT ||
                        item.second.get_property_type() == wanda_property_types::COV ||
                        item.second.get_property_type() == wanda_property_types::COS)
                    {
                        std::vector<int> OutputIndices(N_group);
                        if (output_index_group_cache.find(item.second.get_wdo_postfix()) ==
                            output_index_group_cache.end())
                        { // this index hasn't been found yet.
                            wanda_output_file.get_int_element("C_COMP_INDEX", "Frst_chn_ndx", {1, N_group, 1},
                                                              OutputIndices);
                            output_index_group_cache["CHANNEL"] = OutputIndices;
                            wanda_output_file.get_int_element("C_COMP_INDEX", "Frst_cos_ndx", {1, N_group, 1},
                                                              OutputIndices);
                            output_index_group_cache["COS"] = OutputIndices;
                            wanda_output_file.get_int_element("C_COMP_INDEX", "Frst_cov_ndx", {1, N_group, 1},
                                                              OutputIndices);
                            output_index_group_cache["COV"] = OutputIndices;
                            // only 3 possible WDO postfixes for Control components
                        }

                        OutputIndices = output_index_group_cache[item.second.get_wdo_postfix()];

                        int KeyIndex = ctrl__index[comp.first];
                        item.second.set_group_index(OutputIndices[KeyIndex]); //-1 to adjust for NEFIS 1-based
                                                                              // array indexing
                    }
                }
            }
        }
    }
}

// private method
int wanda_model::get_key_index_array(std::vector<std::string> KeyArray, std::string key)
{
    ptrdiff_t pos = std::find(KeyArray.begin(), KeyArray.end(), key) - KeyArray.begin();
    if (pos >= int(KeyArray.size()))
    {
        return -1;
    }
    return static_cast<int>(pos);
}

// private method
std::vector<int> wanda_model::get_key_indices_array(std::vector<std::string> KeyArray, std::string key)
{
    std::vector<int> KeyIndex;
    for (auto i = 0; i < int(KeyArray.size()); i++)
    {
        if (KeyArray[i].compare((key)) == 0)
        {
            KeyIndex.push_back(i);
        }
    }
    return KeyIndex;
}

// private method
std::string wanda_model::to_lower(std::string in) const
{
    std::transform(in.begin(), in.end(), in.begin(), ::tolower);
    return in;
}

wanda_property &wanda_model::get_property(std::string PropertyDescription)
{
    if (global_vars.find(PropertyDescription) != global_vars.end())
    {
        return global_vars[PropertyDescription];
    }
    if (mode_and_opt.find(PropertyDescription) != mode_and_opt.end())
    {
        return mode_and_opt[PropertyDescription];
    }
    throw std::invalid_argument(PropertyDescription + " does not exist in WandaModel object");
}

wanda_component &wanda_model::get_component(const std::string &componentname)
{
    if (name2_phys_comp.find(componentname) != name2_phys_comp.end())
    {
        return *name2_phys_comp.at(componentname);
    }
    for (auto &comp : phys_components)
    {
        if (comp.second.get_complete_name_spec() == componentname)
        {
            return comp.second;
        }
    }
    for (auto &comp : ctrl_components)
    {
        if (comp.second.get_complete_name_spec() == componentname)
        {
            return comp.second;
        }
    }
    throw std::invalid_argument(componentname + "does not exist in WandaModel object");
}

wanda_node &wanda_model::get_node(const std::string &nodename)
{
    if (name2_phys_node.find(nodename) != name2_phys_node.end())
    {
        return *name2_phys_node.at(nodename);
    }
    throw std::invalid_argument(nodename + "does not exist in WandaModel object");
}

//! Gets a node from the wanda_model
/*!
* get_signal_line() returns a wanda_sig_line object that represents the
* requested signal line
\param sig_name Name of the requested signal line
*/
wanda_sig_line &wanda_model::get_signal_line(const std::string &sig_name)
{
    for (auto &sigline : signal_lines)
    {
        if (sigline.second.get_complete_name_spec() == sig_name)
        {
            return sigline.second;
        }
    }
    throw std::invalid_argument(sig_name + " not found");
}

// private method
wanda_item &wanda_model::connect_sensor(wanda_component &h_comp1, int con_point1, wanda_component &sensor,
                                        int con_point2)
{
    // component1 is a physical comp, comp 2 is a sensor
    // check if they are already connected on the given connectpoints

    if (sensor.is_sigline_connected(con_point2, true))
    {
        throw std::runtime_error(sensor.get_complete_name_spec() + " is already connected to a component");
    }

    auto position1 = h_comp1.get_position();
    auto position2 = sensor.get_position();
    float newx = (position1[0] + position2[0]) / 2; // setting the new signal line right between the components
                                                    // it is connecting so that the model in the UI looks ok.
    float newy = (position1[1] + position2[1]) / 2;
    std::vector<float> new_position = {newx, newy};
    wanda_sig_line &new_sig_line = add_sigline("sensor", new_position);

    h_comp1.connect(new_sig_line, con_point1, false);
    sensor.connect(new_sig_line, con_point2, true);
    new_sig_line.connect_output(h_comp1, con_point1);
    new_sig_line.connect_input(sensor, con_point2);
    return new_sig_line;
}

void wanda_model::connect_sensor(wanda_node &node, wanda_component &sensor)
{
    // check if they are already connected on the given connectpoints

    if (sensor.is_node_connected(1))
    {
        throw std::runtime_error(sensor.get_complete_name_spec() + " is already connected to a component");
    }

    auto position1 = node.get_position();
    auto position2 = sensor.get_position();

    node.connect(sensor);
    sensor.connect(node, 1);
}

// private method
void wanda_model::save_unit_system()
{
    // saving unit system
    std::vector<std::string> un_group(1);
    un_group[0] = unit_group;
    wanda_input_file.write_string_elements("CASE_INFORMATION", "Unit_group", nefis_file::single_elem_uindex, 16,
                                           un_group);
    if (unit_group.compare("UNIT_GROUP_USER") == 0)
    {
        std::vector<std::string> case_unit_descr;
        std::vector<std::string> case_unit_dim;
        for (auto item : case_units)
        {
            case_unit_descr.push_back(item.second);
            case_unit_dim.push_back(item.first);
        }

        for (int i = case_unit_descr.size(); i < 100; i++)
        {
            case_unit_descr.push_back("");
            case_unit_dim.push_back("");
        }

        wanda_input_file.write_string_elements("CASE_INFORMATION", "User_unit_descr", nefis_file::single_elem_uindex,
                                               16, case_unit_descr);
        wanda_input_file.write_string_elements("CASE_INFORMATION", "User_unit_dims", nefis_file::single_elem_uindex, 12,
                                               case_unit_dim);
    }
}

// private method
void wanda_model::save_glob_vars()
{
    // getting list of globvar
    globvar_base_data globvars;

    // saving of the globvars
    for (auto element : globvars.get_data_as_list())
    {
        if (element.type != "REAL" && element.type != "INTEGER" && element.type != "TABLE" && element.type != "CHOICE")
        {
            continue;
        }
        if (global_vars[element.name].is_modified() || new_case_statusflag)
        {
            if (element.type == "REAL")
            {
                std::vector<float> fltResult(element.elsize);
                wanda_input_file.get_float_element(element.groupname, element.element_name,
                                                   nefis_file::single_elem_uindex, fltResult);
                fltResult[element.elnum - 1] = global_vars[element.name].get_scalar_float();
                wanda_input_file.write_float_elements(element.groupname, element.element_name,
                                                      nefis_file::single_elem_uindex, fltResult);
            }
            if (element.type == "INTEGER" || element.type == "CHOICE")
            {
                std::vector<int> intResult(element.elsize);
                wanda_input_file.get_int_element(element.groupname, element.element_name,
                                                 nefis_file::single_elem_uindex, intResult);
                float result = global_vars[element.name].get_scalar_float();
                intResult[element.elnum - 1] = int(result);
                wanda_input_file.write_int_elements(element.groupname, element.element_name,
                                                    nefis_file::single_elem_uindex, intResult);
            }
            if (element.type == "TABLE" && element.name == "Temp.dep. properties")
            {
                save_table(global_vars.at(element.name).get_table());
                auto descr = globvars.get_temp_dep_fluid_descr().at(element.element_name);
                std::vector<std::string> key;
                key.push_back(global_vars.at(element.name).get_table().get_key(descr));
                wanda_input_file.write_string_elements(element.groupname, element.element_name,
                                                       nefis_file::single_elem_uindex, 8, key);
            }
        }
    }
}

// private method
bool wanda_model::save_new_phys_comp()
{
    bool remove_wdx = false;
    int numrecords = wanda_input_file.get_maxdim_index("H_COMPONENTS");

    std::vector<std::string> comp_keys(numrecords);
    std::vector<std::string> spec_oper_keys(numrecords);

    std::vector<int> free_comp_rec;
    std::vector<int> free_ope_rec;
    std::vector<int> free_com_rec;
    if (numrecords != 0)
    {
        wanda_input_file.get_string_element("H_COMPONENTS", "H_comp_key", {1, numrecords, 1}, 8, comp_keys);
        wanda_input_file.get_string_element("H_COMPONENTS", "Spec_oper_key", {1, numrecords, 1}, 8, spec_oper_keys);
        int erased = 0;
        do
        {
            free_comp_rec.push_back(get_key_index_array(comp_keys, "Free"));
            if (free_comp_rec.back() != -1)
            {
                comp_keys.erase(comp_keys.begin() + free_comp_rec.back());
                spec_oper_keys.erase(spec_oper_keys.begin() + free_comp_rec.back());
                free_comp_rec.back() += erased;
                erased++;
            }
        } while (free_comp_rec.back() != -1);
    }
    else
    {
        free_comp_rec.push_back(-1);
    }

    int numrecords_ope = wanda_input_file.get_maxdim_index("H_OPE_SPEC_VAL");
    std::vector<std::string> ope_spec_oper_key(numrecords_ope);
    std::vector<std::string> ope_spec_com_keys(numrecords_ope);
    if (numrecords_ope != 0)
    {
        wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_oper_key", {1, numrecords_ope, 1}, 8,
                                            ope_spec_oper_key);
        wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_comm_key", {1, numrecords_ope, 1}, 8,
                                            ope_spec_com_keys);
        int erased = 0;
        do
        {
            free_ope_rec.push_back(get_key_index_array(ope_spec_oper_key, "Free"));
            if (free_ope_rec.back() != -1)
            {
                ope_spec_oper_key.erase(ope_spec_oper_key.begin() + free_ope_rec.back());
                ope_spec_com_keys.erase(ope_spec_com_keys.begin() + free_ope_rec.back());
                free_ope_rec.back() += erased;
                erased++;
            }
        } while (free_ope_rec.back() != -1);
    }
    else
    {
        free_ope_rec.push_back(-1);
    }

    int numrecords_com = wanda_input_file.get_maxdim_index("H_COM_SPEC_VAL");
    std::vector<std::string> spec_com_keys(numrecords_com);
    if (numrecords_com != 0)
    {
        wanda_input_file.get_string_element("H_COM_SPEC_VAL", "Spec_comm_key", {1, numrecords_com, 1}, 8,
                                            spec_com_keys);
        int erased = 0;
        do
        {
            free_com_rec.push_back(get_key_index_array(spec_com_keys, "Free"));
            if (free_com_rec.back() != -1)
            {
                spec_com_keys.erase(spec_com_keys.begin() + free_com_rec.back());
                free_com_rec.back() += erased;
                erased++;
            }
        } while (free_com_rec.back() != -1);
    }
    else
    {
        free_com_rec.push_back(-1);
    }

    for (auto &temp : phys_components)
    {
        auto &comp = temp.second;
        if (comp.is_new())
        {
            remove_wdx = true;

            int rec = free_comp_rec[0]; // get_key_index_array(comp_keys, "Free");
            if (rec == -1)
            {
                rec = numrecords;
                numrecords++;
                comp_keys.push_back(comp.get_key_as_string());
            }
            else
            {
                free_comp_rec.erase(free_comp_rec.begin());
            }
            // comp_keys[rec] = comp.get_key_as_string();
            rec++;                  // nefis index corection, or add one in case rec=numrecords so we
                                    // have a new record
            comp.set_comp_num(rec); // Save nefis record index in component

            nefis_uindex uindex = {rec, rec, 1};
            std::vector<std::string> comp_key(1);
            comp_key[0] = comp.get_key_as_string();
            wanda_input_file.write_string_elements("H_COMPONENTS", "H_comp_key", uindex, 8, comp_key);
            std::vector<std::string> class_name;
            class_name.push_back(comp.get_class_sort_key());
            wanda_input_file.write_string_elements("H_COMPONENTS", "Class_name", uindex, 8, class_name);
            wanda_input_file.write_string_elements("H_COMPONENTS", "Class_sort_key", uindex, 8, class_name);
            std::vector<std::string> comp_type;
            comp_type.push_back(comp.get_physcomp_type());
            wanda_input_file.write_string_elements("H_COMPONENTS", "Comp_type", uindex, 8, comp_type);
            std::vector<int> flip_horizontal;
            flip_horizontal.push_back(0);
            wanda_input_file.write_int_elements("H_COMPONENTS", "Flip_horizontal", uindex, flip_horizontal);
            std::vector<int> flip_vertical;
            flip_vertical.push_back(0);
            wanda_input_file.write_int_elements("H_COMPONENTS", "Flip_vertical", uindex, flip_vertical);
            std::vector<int> has_dynamic_text;
            has_dynamic_text.push_back(0);
            wanda_input_file.write_int_elements("H_COMPONENTS", "Has_dynamic_text", uindex, has_dynamic_text);
            std::vector<int> inputready_flags;
            inputready_flags.push_back(0);
            wanda_input_file.write_int_elements("H_COMPONENTS", "Inputready_flags", uindex, inputready_flags);
            std::vector<int> is_valid_connect;
            is_valid_connect.push_back(0);
            wanda_input_file.write_int_elements("H_COMPONENTS", "Is_valid_connect", uindex, is_valid_connect);
            std::vector<std::string> comp_sp_inp_stat;
            comp_sp_inp_stat.push_back("H");
            wanda_input_file.write_string_elements("H_COMPONENTS", "Comp_sp_inp_stat", uindex, 1, comp_sp_inp_stat);

            std::vector<float> centrpos = comp.get_position();
            wanda_input_file.write_float_elements("H_COMPONENTS", "Comp_centre_pos", uindex, centrpos);

            // isvisb elements are skipped since they are not used
            std::string spc_ope_key = get_unique_key(spec_oper_keys, 'O', last_key);
            std::vector<std::string> spec_ope_keys;
            spec_ope_keys.push_back(spc_ope_key);
            wanda_input_file.write_string_elements("H_COMPONENTS", "Spec_oper_key", uindex, 8, spec_ope_keys);

            int ope_rec = free_ope_rec[0]; // get_key_index_array(ope_spec_oper_key, "Free");
            if (ope_rec == -1)
            {
                ope_rec = numrecords_ope;
                numrecords_ope++;
            }
            else
            {
                free_ope_rec.erase(free_ope_rec.begin());
            }
            // ope_spec_oper_key[ope_rec] = spc_ope_key;
            ope_rec++; // nefis index corection, or add one in case rec=numrecords so
                       // we have a new record
            comp.set_oper_index(ope_rec);
            nefis_uindex ope_rec_uindex = {ope_rec, ope_rec, 1};
            wanda_input_file.write_string_elements("H_OPE_SPEC_VAL", "Spec_oper_key", ope_rec_uindex, 8, spec_ope_keys);

            std::string spc_com_key = get_unique_key(ope_spec_com_keys, 'C', last_key);
            std::vector<std::string> spc_com_keys;
            spc_com_keys.push_back(spc_com_key);
            wanda_input_file.write_string_elements("H_OPE_SPEC_VAL", "Spec_comm_key", ope_rec_uindex, 8, spc_com_keys);
            wanda_input_file.write_string_elements("H_OPE_SPEC_VAL", "Class_sort_key", ope_rec_uindex, 8, class_name);

            int com_rec = free_com_rec[0]; // get_key_index_array(spec_com_keys, "Free");
            if (com_rec == -1)
            {
                com_rec = numrecords_com;
                numrecords_com++;
                spec_com_keys.push_back(spc_com_key);
            }
            else
            {
                free_com_rec.erase(free_com_rec.begin());
            }
            // spec_com_keys[com_rec] = spc_com_key;
            com_rec++; // nefis index corection
            comp.set_com_index(com_rec);
            nefis_uindex com_rec_uindex = {com_rec, com_rec, 1};
            wanda_input_file.write_string_elements("H_COM_SPEC_VAL", "Spec_comm_key", com_rec_uindex, 8, spc_com_keys);
            wanda_input_file.write_string_elements("H_COM_SPEC_VAL", "Class_sort_key", com_rec_uindex, 8, class_name);
            std::vector<int> nhis = component_definition->get_num_input_props(comp.get_class_sort_key());
            std::vector<int> N_his_com;
            N_his_com.push_back(nhis[0]);
            wanda_input_file.write_int_elements("H_COM_SPEC_VAL", "N_his", com_rec_uindex, N_his_com);
            std::vector<int> N_his_ope;
            N_his_ope.push_back(nhis[1]);
            wanda_input_file.write_int_elements("H_OPE_SPEC_VAL", "N_his", ope_rec_uindex, N_his_ope);
            std::vector<int> N_hcs;
            N_hcs.push_back(nhis[2]);
            wanda_input_file.write_int_elements("H_OPE_SPEC_VAL", "N_hcs", ope_rec_uindex, N_hcs);

            std::vector<std::string> conn_line_keys;
            conn_line_keys.push_back(unref);
            conn_line_keys.push_back(unref);
            wanda_input_file.write_string_elements("H_COMPONENTS", "Conn_line_keys", uindex, 8, conn_line_keys);
            std::vector<std::string> spec_ip_fld_his_com(36);
            std::vector<std::string> spec_ip_fld_his_ope(36);
            for (auto &prop : comp)
            {
                if (prop.second.is_input())
                {
                    if (prop.second.get_property_spec_code() == 'C')
                    {
                        spec_ip_fld_his_com[prop.second.get_index()] = prop.second.get_property_spec_inp_fld();
                        // Changing Integer to real otherwise UI and steady and unsteady cannot cope with it!!
                        // if(spec_ip_fld_his_com[prop.second.get_index()] == "I")
                        // {
                        //   spec_ip_fld_his_com[prop.second.get_index()] = "R";
                        // }
                        if (prop.second.has_table())
                        {
                            auto &table = prop.second.get_table();
                            for (auto description : table.get_descriptions())
                            {
                                spec_ip_fld_his_com[table.get_index(description)] = table.get_table_type(description);
                            }
                        }
                    }
                    if (prop.second.get_property_spec_code() == 'O')
                    {
                        spec_ip_fld_his_ope[prop.second.get_index()] = prop.second.get_property_spec_inp_fld();
                        // if (spec_ip_fld_his_ope[prop.second.get_index()] == "I")
                        // {
                        //    spec_ip_fld_his_ope[prop.second.get_index()] = "R";
                        // }
                        if (prop.second.has_table())
                        {
                            auto &table = prop.second.get_table();
                            for (auto description : table.get_descriptions())
                            {
                                spec_ip_fld_his_ope[table.get_index(description)] = table.get_table_type(description);
                            }
                        }
                    }
                }
            }

            wanda_input_file.write_string_elements("H_COM_SPEC_VAL", "Spec_ip_fld_his", com_rec_uindex, 1,
                                                   spec_ip_fld_his_com);
            wanda_input_file.write_string_elements("H_OPE_SPEC_VAL", "Spec_ip_fld_his", ope_rec_uindex, 1,
                                                   spec_ip_fld_his_ope);

            wanda_input_file.set_int_attribute("H_COMPONENTS", "N_group", number_physical_components);
            wanda_input_file.set_int_attribute("H_COM_SPEC_VAL", "N_group", number_physical_components);
            wanda_input_file.set_int_attribute("H_OPE_SPEC_VAL", "N_group", number_physical_components);
        }
    }
    return remove_wdx;
}

// private method
void wanda_model::save_new_ctrl_comp(wanda_component &comp)
{
    int numrecords = wanda_input_file.get_maxdim_index("C_COMPONENTS");
    std::vector<std::string> comp_keys(numrecords);

    if (numrecords != 0)
    {
        wanda_input_file.get_string_element("C_COMPONENTS", "C_comp_key", {1, numrecords, 1}, 8, comp_keys);
    }

    int rec = get_key_index_array(comp_keys, "Free");
    if (rec == -1)
        rec = numrecords;
    rec++; // nefis index corection, or add one in case rec=numrecords so we have
           // a new record
    comp.set_comp_num(rec);
    std::vector<std::string> comp_key(1);
    comp_key[0] = comp.get_key_as_string();
    nefis_uindex uindex = {rec, rec, 1};
    wanda_input_file.write_string_elements("C_COMPONENTS", "C_comp_key", uindex, 8, comp_key);
    std::vector<std::string> class_name;
    class_name.push_back(comp.get_class_sort_key());
    wanda_input_file.write_string_elements("C_COMPONENTS", "Class_name", uindex, 8, class_name);
    std::vector<std::string> comp_type;
    if (comp.get_class_name() == "SENSOR")
    {
        comp_type.push_back("HYDR_SEN");
    }
    else
    {
        comp_type.push_back("CTR");
    }
    wanda_input_file.write_string_elements("C_COMPONENTS", "Comp_type", uindex, 8, comp_type);
    std::vector<int> flip_horizontal;
    flip_horizontal.push_back(0);
    wanda_input_file.write_int_elements("C_COMPONENTS", "Flip_horizontal", uindex, flip_horizontal);
    std::vector<int> flip_vertical;
    flip_vertical.push_back(0);
    wanda_input_file.write_int_elements("C_COMPONENTS", "Flip_vertical", uindex, flip_vertical);
    std::vector<int> has_dynamic_text;
    has_dynamic_text.push_back(0);
    wanda_input_file.write_int_elements("C_COMPONENTS", "Has_dynamic_text", uindex, has_dynamic_text);
    std::vector<int> inputready_flags;
    inputready_flags.push_back(0);
    wanda_input_file.write_int_elements("C_COMPONENTS", "Inputready_flags", uindex, inputready_flags);
    std::vector<int> is_valid_connect;
    is_valid_connect.push_back(0);
    wanda_input_file.write_int_elements("C_COMPONENTS", "Is_valid_connect", uindex, is_valid_connect);
    std::vector<std::string> comp_sp_inp_stat;
    comp_sp_inp_stat.push_back("C");
    wanda_input_file.write_string_elements("C_COMPONENTS", "Comp_sp_inp_stat", uindex, 1, comp_sp_inp_stat);

    std::vector<float> centrpos = comp.get_position();
    wanda_input_file.write_float_elements("C_COMPONENTS", "Comp_touch_pos", uindex, centrpos);
    std::vector<std::string> spec_ip_fld_cis;
    component_definition->get_ip_fld_his(comp.get_class_sort_key(), spec_ip_fld_cis);
    wanda_input_file.write_string_elements("C_COMPONENTS", "Spec_ip_fld_cis", uindex, 1, spec_ip_fld_cis);
    std::vector<int> n_cis;
    n_cis.push_back(spec_ip_fld_cis.size());
    wanda_input_file.write_int_elements("C_COMPONENTS", "N_cis", uindex, {static_cast<int>(spec_ip_fld_cis.size())});
    numrecords++;
    wanda_input_file.set_int_attribute("C_COMPONENTS", "N_group", numrecords);
    std::vector<std::string> chanl_out_type(16);
    for (int i = 1; i <= comp.get_num_output_channels(); i++)
    {
        chanl_out_type[i - 1] = comp.get_output_channel_type(i);
    }
    wanda_input_file.write_string_elements("C_COMPONENTS", "Chanl_out_type", uindex, 8, chanl_out_type);
    std::vector<int> n_out_channel;
    n_out_channel.push_back(comp.get_num_output_channels());
    wanda_input_file.write_int_elements("C_COMPONENTS", "N_output_chanl", uindex, n_out_channel);
    std::vector<int> n_input_channel;
    n_input_channel.push_back(comp.get_num_input_channels());
    wanda_input_file.write_int_elements("C_COMPONENTS", "N_input_chanl", uindex, n_input_channel);
}

// private method
bool wanda_model::save_new_node()
{
    bool remove_wdx = false;
    int numrecords = wanda_input_file.get_maxdim_index("H_NODES");
    std::vector<std::string> h_node_key(numrecords);
    std::vector<int> free_record;

    if (numrecords != 0)
    {
        wanda_input_file.get_string_element("H_NODES", "H_node_key", {1, numrecords, 1}, 8, h_node_key);

        int erased = 0;
        do
        {
            free_record.push_back(get_key_index_array(h_node_key, "Free"));
            if (free_record.back() != -1)
            {
                h_node_key.erase(h_node_key.begin() + free_record.back());
                free_record.back() += erased;
                erased++;
            }
        } while (free_record.back() != -1);
    }
    else
    {
        free_record.push_back(-1);
    }

    for (auto &item : phys_nodes)
    {
        auto &node = item.second;
        if (!node.is_new())
        {
            continue;
        }
        remove_wdx = true;
        int rec = free_record[0]; // get_key_index_array(h_node_key, "Free");
        if (rec == -1)
        {
            rec = numrecords;
            h_node_key.push_back(node.get_key_as_string());
            numrecords++;
        }
        else
        {
            free_record.erase(free_record.begin());
        }
        node.set_group_index(rec);
        // h_node_key[rec] = node.get_key_as_string();
        rec++; // nefis index corection, or add one in case rec=numrecords so we
               // have a new record
        std::vector<std::string> class_name = {node.get_class_sort_key()};
        nefis_uindex uindex = {rec, rec, 1};
        wanda_input_file.write_string_elements("H_NODES", "Class_name", uindex, 8, class_name);
        std::vector<std::string> h_comp_key;
        h_comp_key.push_back(node.get_key_as_string());
        wanda_input_file.write_string_elements("H_NODES", "H_node_key", uindex, 8, h_comp_key);
        std::vector<float> abs_pos = node.get_position();
        wanda_input_file.write_float_elements("H_NODES", "Abs_position", uindex, abs_pos);
        // is valid conected
        std::vector<int> is_val_con(1);
        is_val_con[0] = 1;
        wanda_input_file.write_int_elements("H_NODES", "Is_valid_connect", uindex, is_val_con);

        // n his
        std::vector<int> n_nis = component_definition->get_num_input_props(node.get_class_sort_key());
        wanda_input_file.write_int_elements("H_NODES", "N_nis", uindex, n_nis);

        // specchrval nis
        // spec_ip_flid_nis
        std::vector<std::string> spec_ip_fld_nis =
            component_definition->get_comp_spec_ip_fld(node.get_class_sort_key());
        wanda_input_file.write_string_elements("H_NODES", "Spec_ip_fld_nis", uindex, 1, spec_ip_fld_nis);
        node.set_new(false);
    }
    numrecords++;
    wanda_input_file.set_int_attribute("H_NODES", "N_group", numrecords);
    return remove_wdx;
}

// private method
void wanda_model::save_new_sig_line(wanda_sig_line &sig_line)
{
    int numrecords = wanda_input_file.get_maxdim_index("SIGNAL_LINES");
    std::vector<std::string> h_node_key(numrecords);
    if (numrecords != 0)
    {
        wanda_input_file.get_string_element("SIGNAL_LINES", "Sig_line_key", {1, numrecords, 1}, 8, h_node_key);
    }
    int rec = numrecords + 1;
    if (numrecords != 0)
    {
        rec = get_key_index_array(h_node_key, "Free") + 1;
        if (rec == 0)
            rec = numrecords + 1;
    }
    std::vector<std::string> h_comp_key = {sig_line.get_key_as_string()};
    nefis_uindex uindex = {rec, rec, 1};
    wanda_input_file.write_string_elements("SIGNAL_LINES", "Sig_line_key", uindex, 8, h_comp_key);
    // is valid conected
    std::vector<int> is_val_con;
    is_val_con.push_back(1);
    wanda_input_file.write_int_elements("SIGNAL_LINES", "Is_valid_connect", uindex, is_val_con);
    sig_line.set_new(false);
}

void wanda_model::save_phys_comp_input()
{
    int numrecords = wanda_input_file.get_maxdim_index("H_COMPONENTS");
    if (numrecords == 0)
    {
        return;
    }
    nefis_uindex all_records_uindex = {1, numrecords, 1};

    std::vector<std::string> comp_keys(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "H_comp_key", all_records_uindex, 8, comp_keys);
    std::vector<std::string> spec_oper_keys(numrecords);
    wanda_input_file.get_string_element("H_COMPONENTS", "Spec_oper_key", all_records_uindex, 8, spec_oper_keys);

    int numrecords_ope = wanda_input_file.get_maxdim_index("H_OPE_SPEC_VAL");
    std::vector<std::string> ope_spec_oper_key(numrecords_ope);
    wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_oper_key", {1, numrecords_ope, 1}, 8,
                                        ope_spec_oper_key);
    std::vector<std::string> ope_spec_com_keys(numrecords_ope);
    wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_comm_key", {1, numrecords_ope, 1}, 8,
                                        ope_spec_com_keys);

    int numrecords_com = wanda_input_file.get_maxdim_index("H_COM_SPEC_VAL");
    std::vector<std::string> spec_com_keys(numrecords_com);
    wanda_input_file.get_string_element("H_COM_SPEC_VAL", "Spec_comm_key", {1, numrecords_com, 1}, 8, spec_com_keys);

    for (auto &item : phys_components)
    {
        auto &comp = item.second;

        if (comp.is_pipe())
        {
            calc_hsc(comp);            
        }

        int rec = comp.get_comp_num();       // get_key_index_array(comp_keys,
                                             // comp.get_key_as_string()) + 1;
        int ope_rec = comp.get_oper_index(); // get_key_index_array(ope_spec_oper_key,
                                             // spec_oper_keys[rec - 1]) + 1;
        int com_rec = comp.get_com_index();  // get_key_index_array(spec_com_keys,
                                             // ope_spec_com_keys[ope_rec - 1]) + 1;
        nefis_uindex rec_uindex = {rec, rec, 1};
        nefis_uindex ope_rec_uindex = {ope_rec, ope_rec, 1};
        nefis_uindex com_rec_uindex = {com_rec, com_rec, 1};

        if (comp.is_modified())
        {
            std::vector<int> color;
            color.push_back(0);
            wanda_input_file.write_int_elements("H_COMPONENTS", "Color", rec_uindex, color);

            std::vector<std::string> comment;
            comment.push_back(comp.get_comment());
            wanda_input_file.write_string_elements("H_COMPONENTS", "Comment", rec_uindex, 50, comment);

            std::vector<std::string> date_time_modify;
            date_time_modify.push_back(comp.get_date_mod());
            wanda_input_file.write_string_elements("H_COMPONENTS", "Date_time_modify", rec_uindex, 17,
                                                   date_time_modify);

            std::vector<int> is_disused;
            is_disused.push_back(comp.is_disused());
            wanda_input_file.write_int_elements("H_COMPONENTS", "Is_disused", rec_uindex, is_disused);

            std::vector<std::string> keywords;
            std::string keyword_line = keywords2_list(comp.get_keywords());
            keywords.push_back(keyword_line);
            wanda_input_file.write_string_elements("H_COMPONENTS", "Keywords", rec_uindex, 50, keywords);

            std::vector<float> centrpos = comp.get_position();
            wanda_input_file.write_float_elements("H_COMPONENTS", "Comp_centre_pos", rec_uindex, centrpos);

            std::vector<std::string> material_name;
            material_name.push_back(comp.get_material_name());
            wanda_input_file.write_string_elements("H_COMPONENTS", "Material_name", rec_uindex, 24, material_name);
            std::vector<std::string> model_name;
            model_name.push_back(comp.get_model_name());
            wanda_input_file.write_string_elements("H_COMPONENTS", "Model_name", rec_uindex, 24, model_name);

            std::vector<int> n_elements;
            n_elements.push_back(comp.get_num_elements());
            wanda_input_file.write_int_elements("H_COMPONENTS", "N_elements", rec_uindex, n_elements);

            std::vector<std::string> name;
            name.push_back(comp.get_name());
            wanda_input_file.write_string_elements("H_COMPONENTS", "Name", rec_uindex, 0, name);
            // skipped name color, position, prefix

            // skipped pipe key
            std::vector<std::string> reference_id;
            reference_id.push_back(comp.get_ref_id());
            wanda_input_file.write_string_elements("H_COMPONENTS", "Reference_id", rec_uindex, 120, reference_id);
            std::vector<int> RGB_color;
            RGB_color.push_back(0);
            wanda_input_file.write_int_elements("H_COMPONENTS", "RGB_color", rec_uindex, RGB_color);
            std::vector<float> rotate_angle;
            rotate_angle.push_back(comp.get_angle_rad());
            wanda_input_file.write_float_elements("H_COMPONENTS", "Rotate_angle", rec_uindex, rotate_angle);
            // sort name, is skipped since it is not correct in the WDI file
            std::vector<int> sort_sequence;
            sort_sequence.push_back(comp.get_sequence_number());
            wanda_input_file.write_int_elements("H_COMPONENTS", "Sort_sequence", rec_uindex, sort_sequence);
            std::vector<int> use_action_table;
            if (comp.has_action_table())
            {
                use_action_table.push_back(comp.is_action_table_used() ? 1 : 0);
                wanda_input_file.write_int_elements("H_COMPONENTS", "Use_action_table", rec_uindex, use_action_table);
            }

            std::vector<std::string> user_name;
            user_name.push_back(comp.get_user_name());
            wanda_input_file.write_string_elements("H_COMPONENTS", "User_name", rec_uindex, 24, user_name);
            std::vector<std::string> nodes(4);
            for (int i = 1; i <= 4; i++)
            {
                if (comp.is_node_connected(i))
                {
                    nodes[i - 1] = static_cast<wanda_node &>(comp.get_connected_node(i)).get_key_as_string();
                }
                else
                {
                    nodes[i - 1] = unref;
                }
            }
            wanda_input_file.write_string_elements("H_COMPONENTS", "H_node_keys", rec_uindex, 8, nodes);
        }

        // saving input data
        std::vector<float> ope_spec_numval_his(36);
        std::vector<float> com_spec_numval_his(36);
        std::vector<std::string> ope_spec_charval_his(36);
        std::vector<std::string> com_spec_charval_his(36);
        std::vector<std::string> ope_spec_status_his(36);
        std::vector<std::string> com_spec_status_his(36);
        bool newcomp = comp.is_new();
        comp.set_new(false);
        if (!newcomp)
        {
            wanda_input_file.get_float_element("H_OPE_SPEC_VAL", "Spec_numval_his", ope_rec_uindex,
                                               ope_spec_numval_his);
            wanda_input_file.get_float_element("H_COM_SPEC_VAL", "Spec_numval_his", com_rec_uindex,
                                               com_spec_numval_his);
            wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_chrval_his", ope_rec_uindex, 16,
                                                ope_spec_charval_his);
            wanda_input_file.get_string_element("H_COM_SPEC_VAL", "Spec_chrval_his", com_rec_uindex, 16,
                                                com_spec_charval_his);
            wanda_input_file.get_string_element("H_OPE_SPEC_VAL", "Spec_status_his", ope_rec_uindex, 1,
                                                ope_spec_status_his);
            wanda_input_file.get_string_element("H_COM_SPEC_VAL", "Spec_status_his", com_rec_uindex, 1,
                                                com_spec_status_his);
        }
        bool modified = false;
        for (auto it = comp.begin(); it != comp.end(); ++it)
        {
            auto &prop = it->second;
            if (!prop.is_modified() || prop.get_property_type() != wanda_property_types::HIS)
            {
                if (!prop.has_table())
                {
                    continue;
                }

                if (!prop.get_table().is_modified())
                {
                    continue;
                }
            }
            if (prop.get_description() == "Profile")
            {
                calc_hsc(comp);                
            }
            modified = true;
            if (prop.get_property_spec_code() == 'O')
            {
                ope_spec_status_his[prop.get_index()] = "-";
                if (prop.get_property_spec_inp_fld() == 'T' || prop.get_property_spec_inp_fld() == 'S' ||
                    prop.get_property_spec_inp_fld() == 'N')
                {
                    // first save the tables since this creates the table keys
                    if (prop.get_table().is_modified())
                    {
                        save_table(prop.get_table());
                        for (auto description : prop.get_table().get_descriptions())
                        {
                            wanda_table &table = prop.get_table();
                            if (table.get_spec_code(description) == 'O')
                            {
                                ope_spec_charval_his[table.get_index(description)] = table.get_key(description);
                                ope_spec_status_his[table.get_index(description)] = "H";
                            }
                            else
                            {
                                com_spec_charval_his[table.get_index(description)] = table.get_key(description);
                                com_spec_status_his[table.get_index(description)] = "H";
                            }
                        }
                    }
                }
                else
                {
                    if (prop.get_spec_status())
                    {
                        ope_spec_numval_his[prop.get_index()] = prop.get_scalar_float();
                        ope_spec_status_his[prop.get_index()] = "H";
                    }
                    if (prop.get_property_spec_inp_fld() == 'C')
                    {
                        ope_spec_charval_his[prop.get_index()] = prop.get_selected_item();
                    }
                }
            }

            else if (prop.get_property_spec_code() == 'C')
            {
                if (prop.get_property_spec_inp_fld() == 'T' || prop.get_property_spec_inp_fld() == 'S' ||
                    prop.get_property_spec_inp_fld() == 'N')
                {
                    // first save the tables since this creates the table keys
                    save_table(prop.get_table());
                    for (auto description : prop.get_table().get_descriptions())
                    {
                        wanda_table &table = prop.get_table();
                        ope_spec_status_his[table.get_index(description)] = "-";
                        if (table.get_spec_code(description) == 'O')
                        {
                            ope_spec_charval_his[table.get_index(description)] = table.get_key(description);
                            ope_spec_status_his[table.get_index(description)] = "H";
                        }
                        else
                        {
                            com_spec_charval_his[table.get_index(description)] = table.get_key(description);
                            com_spec_status_his[table.get_index(description)] = "H";
                        }
                    }
                }
                else
                {
                    com_spec_status_his[prop.get_index()] = "-";
                    if (prop.get_spec_status())
                    {
                        com_spec_numval_his[prop.get_index()] = prop.get_scalar_float();
                        com_spec_status_his[prop.get_index()] = "H";
                    }
                    if (prop.get_property_spec_inp_fld() == 'C')
                    {
                        com_spec_charval_his[prop.get_index()] = prop.get_selected_item();
                    }
                }
            }
            else if (prop.get_description() == "Action table")
            {
                std::vector<std::string> action_table_key;
                save_table(prop.get_table());
                action_table_key.push_back(comp.get_property("Action table").get_table().get_key("Time"));
                wanda_input_file.write_string_elements("H_COMPONENTS", "Org_act_tbl_key", rec_uindex, 8,
                                                       action_table_key);
                if (comp.is_action_table_used())
                {
                    wanda_input_file.write_string_elements("H_COMPONENTS", "Action_table_key", rec_uindex, 8,
                                                           action_table_key);
                }
                else
                {
                    action_table_key[0] = unref;
                    wanda_input_file.write_string_elements("H_COMPONENTS", "Action_table_key", rec_uindex, 8,
                                                           action_table_key);
                }
            }
        }
        if (modified)
        {
            wanda_input_file.write_float_elements("H_OPE_SPEC_VAL", "Spec_numval_his", ope_rec_uindex,
                                                  ope_spec_numval_his);
            wanda_input_file.write_float_elements("H_COM_SPEC_VAL", "Spec_numval_his", com_rec_uindex,
                                                  com_spec_numval_his);
            wanda_input_file.write_string_elements("H_OPE_SPEC_VAL", "Spec_chrval_his", ope_rec_uindex, 16,
                                                   ope_spec_charval_his);
            wanda_input_file.write_string_elements("H_COM_SPEC_VAL", "Spec_chrval_his", com_rec_uindex, 16,
                                                   com_spec_charval_his);
            wanda_input_file.write_string_elements("H_OPE_SPEC_VAL", "Spec_status_his", ope_rec_uindex, 1,
                                                   ope_spec_status_his);
            wanda_input_file.write_string_elements("H_COM_SPEC_VAL", "Spec_status_his", com_rec_uindex, 1,
                                                   com_spec_status_his);
        }
    }
}

// private method
void wanda_model::save_ctrl_comp_input(wanda_component &comp)
{
    int numrecords = wanda_input_file.get_maxdim_index("C_COMPONENTS");
    std::vector<std::string> comp_keys(numrecords);
    wanda_input_file.get_string_element("C_COMPONENTS", "C_comp_key", {1, numrecords, 1}, 8, comp_keys);

    int rec = get_key_index_array(comp_keys, comp.get_key_as_string()) + 1;
    nefis_uindex rec_uindex = {rec, rec, 1};
    if (comp.is_modified())
    {
        std::vector<int> color;
        color.push_back(0);
        wanda_input_file.write_int_elements("C_COMPONENTS", "Color", rec_uindex, color);

        std::vector<std::string> comment;
        comment.push_back(comp.get_comment());
        wanda_input_file.write_string_elements("C_COMPONENTS", "Comment", rec_uindex, 50, comment);

        std::vector<std::string> date_time_modify;
        date_time_modify.push_back(comp.get_date_mod());
        wanda_input_file.write_string_elements("C_COMPONENTS", "Date_time_modify", rec_uindex, 17, date_time_modify);

        std::vector<int> is_disused;
        is_disused.push_back(comp.is_disused());
        wanda_input_file.write_int_elements("C_COMPONENTS", "Is_disused", rec_uindex, is_disused);

        std::vector<std::string> keywords;
        std::string keyword_line = keywords2_list(comp.get_keywords());
        keywords.push_back(keyword_line);
        wanda_input_file.write_string_elements("C_COMPONENTS", "Keywords", rec_uindex, 50, keywords);

        std::vector<std::string> name;
        name.push_back(comp.get_name());
        wanda_input_file.write_string_elements("C_COMPONENTS", "Name", rec_uindex, 0, name);
        // skipped name color, position, prefix
        // skipped pipe key
        std::vector<std::string> reference_id;
        reference_id.push_back(comp.get_ref_id());
        wanda_input_file.write_string_elements("C_COMPONENTS", "Reference_id", rec_uindex, 120, reference_id);
        std::vector<int> RGB_color;
        RGB_color.push_back(0);
        wanda_input_file.write_int_elements("C_COMPONENTS", "RGB_color", rec_uindex, RGB_color);
        std::vector<float> rotate_angle;
        rotate_angle.push_back(comp.get_angle_rad());
        wanda_input_file.write_float_elements("C_COMPONENTS", "Rotate_angle", rec_uindex, rotate_angle);
        // sort name, is skipped since it is not correct in the WDI file
        std::vector<int> sort_sequence;
        sort_sequence.push_back(comp.get_sequence_number());
        wanda_input_file.write_int_elements("C_COMPONENTS", "Sort_sequence", rec_uindex, sort_sequence);
        std::vector<std::string> user_name;
        name.push_back(comp.get_user_name());
        wanda_input_file.write_string_elements("C_COMPONENTS", "User_name", rec_uindex, 24, user_name);

        std::vector<std::string> nodes(1);
        std::vector<int> C_comp_hcomp_con(1);
        if (comp.is_node_connected(1))
        {
            nodes[0] = static_cast<wanda_node &>(comp.get_connected_node(1)).get_key_as_string();
        }
        else if (comp.get_type_name() == "Sensor" && comp.is_sigline_connected(1, true))
        {
            auto sigline = comp.get_connected_sigline(1, true)[0];
            nodes[0] = sigline->get_output_component()->get_key_as_string();
            C_comp_hcomp_con[0] = sigline->get_output_component()->get_connect_point(*sigline);
        }
        wanda_input_file.write_string_elements("C_COMPONENTS", "H_comp_key", rec_uindex, 8, nodes);
        wanda_input_file.write_int_elements("C_COMPONENTS", "C_comp_hcomp_con", rec_uindex, C_comp_hcomp_con);
    }
    // saving input data
    std::vector<float> spec_numval_cis(36);
    std::vector<std::string> spec_chrval_cis(36);
    std::vector<std::string> spec_status_cis(36);
    std::vector<int> spec_isvisb_cis(36);
    bool newcomp = comp.is_new();
    if (!newcomp)
    {
        wanda_input_file.get_float_element("C_COMPONENTS", "Spec_numval_cis", rec_uindex, spec_numval_cis);
        wanda_input_file.get_string_element("C_COMPONENTS", "Spec_chrval_cis", rec_uindex, 16, spec_chrval_cis);
        wanda_input_file.get_string_element("C_COMPONENTS", "Spec_status_cis", rec_uindex, 1, spec_status_cis);
        wanda_input_file.get_int_element("C_COMPONENTS", "Spec_isvisb_cis", rec_uindex, spec_isvisb_cis);
    }
    bool modified = false;
    for (auto it = comp.begin(); it != comp.end(); ++it)
    {
        auto prop = it->second;
        if (!prop.is_modified() || prop.get_property_type() != wanda_property_types::CIS)
        {
            if (!prop.has_table())
            {
                continue;
            }
            if (!prop.get_table().is_modified())
            {
                continue;
            }
        }
        modified = true;
        if (prop.get_property_spec_inp_fld() == 'T' || prop.get_property_spec_inp_fld() == 'S' ||
            prop.get_property_spec_inp_fld() == 'N')
        {
            // first save the tables since this creates the table keys
            save_table(prop.get_table());
            for (auto description : prop.get_table().get_descriptions())
            {
                wanda_table &table = prop.get_table();
                spec_chrval_cis[table.get_index(description)] = table.get_key(description);
                spec_status_cis[table.get_index(description)] = "H";
                spec_isvisb_cis[table.get_index(description)] = -1;
            }
        }
        else
        {
            if (prop.get_spec_status())
            {
                spec_numval_cis[prop.get_index()] = prop.get_scalar_float();
                spec_status_cis[prop.get_index()] = "H";

                if (prop.get_property_spec_inp_fld() == 'C')
                {
                    spec_chrval_cis[prop.get_index()] = prop.get_selected_item();
                    spec_isvisb_cis[prop.get_index()] = -1;
                    // If it is a sensor quantity note the name but the short quantity
                    // should be stored in the spec_chrval_cis
                    if (comp.get_class_name() == "SENSOR" && prop.get_description() == "Quantity")
                    {
                        if (comp.is_sigline_connected(1, true))
                        {
                            auto concomp = comp.get_connected_sigline(1, true)[0]->get_output_component();

                            if (concomp->contains_property(prop.get_selected_item()))
                            {
                                auto con_prop = concomp->get_property(prop.get_selected_item());
                                spec_chrval_cis[prop.get_index()] = con_prop.get_short_quant_name();
                            }
                            else
                            { // it is probably a Global quant thus add one to get it
                              // for conneciton point one.
                                // Symbol for other connection poitns will be the same
                                auto con_prop = concomp->get_property(prop.get_selected_item() + " 1");
                                spec_chrval_cis[prop.get_index()] = con_prop.get_short_quant_name();
                            }
                        }
                        else if (comp.is_node_connected(1))
                        {
                            auto connode = comp.get_connected_node(1);
                            auto con_prop = connode.get_property(prop.get_selected_item());
                            spec_chrval_cis[prop.get_index()] = con_prop.get_short_quant_name();
                        }
                    }
                }
            }
        }
    }
    if (modified)
    {
        wanda_input_file.write_float_elements("C_COMPONENTS", "Spec_numval_cis", rec_uindex, spec_numval_cis);
        wanda_input_file.write_string_elements("C_COMPONENTS", "Spec_chrval_cis", rec_uindex, 16, spec_chrval_cis);
        wanda_input_file.write_string_elements("C_COMPONENTS", "Spec_status_cis", rec_uindex, 1, spec_status_cis);
        wanda_input_file.write_int_elements("C_COMPONENTS", "Spec_isvisb_cis", rec_uindex, spec_isvisb_cis);
    }
}

// private method
void wanda_model::save_node_input(wanda_node &node)
{
    // int numrecords = wanda_input_file.get_maxdim_index("H_NODES");
    // std::vector<std::string> node_keys(numrecords);
    // wanda_input_file.get_string_element("H_NODES", "H_node_key", 1, numrecords,
    // 1, 8, node_keys);

    int rec = node.get_group_index() + 1; // get_key_index_array(node_keys,
                                          // node.get_key_as_string()); if (rec == -1)
    nefis_uindex rec_uindex = {rec, rec, 1};
    // rec = numrecords;
    // rec++;// nefis index corection, or add one in case rec=numrecords so we
    // have a new record

    if (node.is_modified())
    {
        std::vector<int> color;
        color.push_back(0);
        wanda_input_file.write_int_elements("H_NODES", "Color", rec_uindex, color);

        std::vector<float> abs_pos = node.get_position();
        wanda_input_file.write_float_elements("H_NODES", "Abs_position", rec_uindex, abs_pos);

        std::vector<std::string> comment;
        comment.push_back(node.get_comment());
        wanda_input_file.write_string_elements("H_NODES", "Comment", rec_uindex, 50, comment);

        std::vector<std::string> date_time_modify;
        date_time_modify.push_back(node.get_date_mod());
        wanda_input_file.write_string_elements("H_NODES", "Date_time_modify", rec_uindex, 17, date_time_modify);

        std::vector<int> is_disused;
        is_disused.push_back(node.is_disused());
        wanda_input_file.write_int_elements("H_NODES", "Is_disused", rec_uindex, is_disused);

        std::vector<std::string> keywords;
        std::string keyword_line = keywords2_list(node.get_keywords());
        keywords.push_back(keyword_line);
        wanda_input_file.write_string_elements("H_NODES", "Keywords", rec_uindex, 50, keywords);

        std::vector<std::string> name;
        name.push_back(node.get_name());
        wanda_input_file.write_string_elements("H_NODES", "Name", rec_uindex, 0, name);
        // skipped name color, position, prefix
        // skipped pipe key
        std::vector<int> RGB_color;
        RGB_color.push_back(0);
        wanda_input_file.write_int_elements("H_NODES", "RGB_color", rec_uindex, RGB_color);

        // get sort name, is skipped since it is not correct in the WDI file
        std::vector<int> sort_sequence;
        sort_sequence.push_back(node.get_sequence_number());
        wanda_input_file.write_int_elements("H_NODES", "Sort_sequence", rec_uindex, sort_sequence);
        std::vector<std::string> user_name;
        name.push_back(node.get_user_name());
        wanda_input_file.write_string_elements("H_NODES", "User_name", rec_uindex, 24, user_name);
    }

    // saving input data
    std::vector<float> spec_numval_nis(36);
    std::vector<std::string> spec_chrval_nos(36);
    std::vector<std::string> spec_status_nis(36);
    wanda_input_file.get_float_element("H_NODES", "Spec_numval_nis", rec_uindex, spec_numval_nis);
    wanda_input_file.get_string_element("H_NODES", "Spec_chrval_nis", rec_uindex, 16, spec_chrval_nos);
    wanda_input_file.get_string_element("H_NODES", "Spec_status_nis", rec_uindex, 1, spec_status_nis);
    bool modified = false;
    for (auto it = node.begin(); it != node.end(); ++it)
    {
        auto prop = it->second;
        if (!prop.is_modified() || prop.get_property_type() != wanda_property_types::NIS)
        {
            if (!prop.has_table())
            {
                continue;
            }
            if (!prop.get_table().is_modified())
            {
                continue;
            }
        }
        modified = true;
        if (prop.get_property_spec_inp_fld() == 'T' || prop.get_property_spec_inp_fld() == 'S' ||
            prop.get_property_spec_inp_fld() == 'N')
        {
            // first save the tables since this creates the table keys
            save_table(prop.get_table());
            for (auto description : prop.get_table().get_descriptions())
            {
                wanda_table &table = prop.get_table();
                spec_chrval_nos[table.get_index(description)] = table.get_key(description);
                spec_status_nis[table.get_index(description)] = "H";
            }
        }
        else
        {
            if (prop.get_spec_status())
            {
                spec_numval_nis[prop.get_index()] = prop.get_scalar_float();
                spec_status_nis[prop.get_index()] = "H";
            }
            if (prop.get_property_spec_inp_fld() == 'C')
            {
                spec_chrval_nos[prop.get_index()] = prop.get_selected_item();
            }
        }
    }
    if (modified)
    {
        wanda_input_file.write_float_elements("H_NODES", "Spec_numval_nis", rec_uindex, spec_numval_nis);
        wanda_input_file.write_string_elements("H_NODES", "Spec_chrval_nis", rec_uindex, 16, spec_chrval_nos);
        wanda_input_file.write_string_elements("H_NODES", "Spec_status_nis", rec_uindex, 1, spec_status_nis);
    }
}

// private method
void wanda_model::save_sig_line_input(wanda_sig_line &sig_lin)
{
    int numrecords = wanda_input_file.get_maxdim_index("SIGNAL_LINES");
    std::vector<std::string> comp_keys(numrecords);
    wanda_input_file.get_string_element("SIGNAL_LINES", "Sig_line_key", {1, numrecords, 1}, 8, comp_keys);

    int rec = get_key_index_array(comp_keys, sig_lin.get_key_as_string()) + 1;
    nefis_uindex rec_uindex = {rec, rec, 1};
    if (sig_lin.is_modified())
    {
        std::vector<int> color;
        color.push_back(0);
        wanda_input_file.write_int_elements("SIGNAL_LINES", "Color", rec_uindex, color);

        std::vector<std::string> comment;
        comment.push_back(sig_lin.get_comment());
        wanda_input_file.write_string_elements("SIGNAL_LINES", "Comment", rec_uindex, 50, comment);

        std::vector<std::string> date_time_modify{sig_lin.get_date_mod()};
        wanda_input_file.write_string_elements("SIGNAL_LINES", "Date_time_modify", rec_uindex, 17, date_time_modify);

        std::vector<int> is_disused = {sig_lin.is_disused()};
        wanda_input_file.write_int_elements("SIGNAL_LINES", "Is_disused", rec_uindex, is_disused);

        std::vector<std::string> keywords{keywords2_list(sig_lin.get_keywords())};
        wanda_input_file.write_string_elements("SIGNAL_LINES", "Keywords", rec_uindex, 50, keywords);

        std::vector<std::string> name{sig_lin.get_name()};
        wanda_input_file.write_string_elements("SIGNAL_LINES", "Name", rec_uindex, 0, name);
        // skipped name color, position, prefix
        // skipped pipe key
        std::vector<int> RGB_color{0};
        wanda_input_file.write_int_elements("SIGNAL_LINES", "RGB_color", rec_uindex, RGB_color);
        std::vector<std::string> user_name{sig_lin.get_user_name()};
        wanda_input_file.write_string_elements("SIGNAL_LINES", "User_name", rec_uindex, 24, user_name);

        std::vector c_comp_keys = {sig_lin.get_output_component()->get_key_as_string(),
                                   sig_lin.get_input_component()->get_key_as_string()};
        wanda_input_file.write_string_elements("SIGNAL_LINES", "C_comp_keys", rec_uindex, 8, c_comp_keys);
        std::vector<int> sig_chnl_ndx{sig_lin.get_output_connection_point(), sig_lin.get_input_connection_point()};
        wanda_input_file.write_int_elements("SIGNAL_LINES", "Sig_chnl_ndx", rec_uindex, sig_chnl_ndx);
        std::vector<std::string> signal_type;
        signal_type.push_back(sig_lin.get_signal_line_type());
        wanda_input_file.write_string_elements("SIGNAL_LINES", "Signal_type", rec_uindex, 8, signal_type);
    }
}

// private method
void wanda_model::load_table()
{
    if (!tables_loaded)
    {
        index_table = wanda_input_file.get_maxdim_index("TABLES");
        if (index_table != 0)
        {
            std::vector<std::string> table_keys_data(index_table);
            std::vector<std::string> _TablesNextKeys(index_table);
            std::vector<int> _table_size(index_table);
            wanda_input_file.get_string_element("TABLES", "Table_key", {1, index_table, 1}, 8, table_keys_data);
            wanda_input_file.get_string_element("TABLES", "Table_key_next", {1, index_table, 1}, 8, _TablesNextKeys);
            wanda_input_file.get_int_element("TABLES", "Table_size", {1, index_table, 1}, _table_size);
            for (int i = 0; i < index_table; i++)
            {
                tabcol_meta_record tab_info;
                tab_info.size = _table_size[i];
                tab_info.table_key_next = _TablesNextKeys[i];
                tab_info.index = i;
                table_metainfo_cache.emplace(table_keys_data[i], tab_info);
            }
        }
        tables_loaded = true;
    }
    if (!num_cols_loaded)
    {
        index_num_col = wanda_input_file.get_maxdim_index("NUMCOLUMNS");
        if (index_num_col != 0)
        {
            std::vector<std::string> num_col_keys(index_num_col);
            std::vector<std::string> _columnKeys_next(index_num_col);
            std::vector<int> _col_size(index_num_col);
            wanda_input_file.get_string_element("NUMCOLUMNS", "Column_key", {1, index_num_col, 1}, 8, num_col_keys);
            wanda_input_file.get_string_element("NUMCOLUMNS", "Column_key_next", {1, index_num_col, 1}, 8,
                                                _columnKeys_next);
            wanda_input_file.get_int_element("NUMCOLUMNS", "Column_size", {1, index_num_col, 1}, _col_size);
            for (int i = 0; i < index_num_col; i++)
            {
                tabcol_meta_record tab_info;
                tab_info.size = _col_size[i];
                tab_info.table_key_next = _columnKeys_next[i];
                tab_info.index = i;
                table_metainfo_cache.emplace(num_col_keys[i], tab_info);
            }
        }
        num_cols_loaded = true;
    }
    if (!string_col_loaded)
    {
        index_string_col = wanda_input_file.get_maxdim_index("CHARCOLUMNS");

        if (index_string_col != 0)
        {
            std::vector<std::string> char_col_keys(index_string_col);
            std::vector<std::string> _columnKeys_next(index_string_col);
            std::vector<int> _col_size(index_string_col);
            wanda_input_file.get_string_element("CHARCOLUMNS", "Column_key", {1, index_string_col, 1}, 8,
                                                char_col_keys);
            wanda_input_file.get_string_element("CHARCOLUMNS", "Column_key_next", {1, index_string_col, 1}, 8,
                                                _columnKeys_next);
            wanda_input_file.get_int_element("CHARCOLUMNS", "Column_size", {1, index_string_col, 1}, _col_size);
            for (int i = 0; i < index_string_col; i++)
            {
                tabcol_meta_record tab_info;
                tab_info.size = _col_size[i];
                tab_info.table_key_next = _columnKeys_next[i];
                tab_info.index = i;
                table_metainfo_cache.emplace(char_col_keys[i], tab_info);
            }
        }
        string_col_loaded = true;
    }
}

std::vector<std::string> wanda_model::get_table_description(const std::string &table_key)
{
    load_table();
    if (table_metainfo_cache.find(table_key) != table_metainfo_cache.end())
    {
        std::vector<std::string> description(2);
        wanda_input_file.get_string_element(
            "TABLES", "Table_descr",
            {table_metainfo_cache.at(table_key).size + 1, table_metainfo_cache.at(table_key).size + 1}, 0, description);
        return description;
    }
    if (table_metainfo_cache.find(table_key) != table_metainfo_cache.end())
    {
        std::vector<std::string> description(1);
        wanda_input_file.get_string_element(
            "NUMCOLUMNS", "Column_descr",
            {table_metainfo_cache.at(table_key).size + 1, table_metainfo_cache.at(table_key).size + 1}, 0, description);
        return description;
    }
    if (table_metainfo_cache.find(table_key) != table_metainfo_cache.end())
    {
        std::vector<std::string> description(1);
        wanda_input_file.get_string_element(
            "CHARCOLUMNS", "Column_descr",
            {table_metainfo_cache.at(table_key).index + 1, table_metainfo_cache.at(table_key).index + 1}, 0,
            description);
        return description;
    }
    throw(std::invalid_argument(table_key + " ot a key in wanda model"));
}

void wanda_model::load_table_data(wanda_table_data *tab_data, std::string &group_name, std::string &element_name)
{
    auto table_metainfo_record = table_metainfo_cache.at(tab_data->_key);
    int index = table_metainfo_record.index + 1;
    int tabsize = table_metainfo_record.size;
    std::vector<float> values(200);
    wanda_input_file.get_float_element(group_name, element_name, {index, index, 1}, values);
    std::vector<float>::const_iterator first = values.begin() + tab_data->_col_num * 100;
    std::vector<float>::const_iterator last = values.begin() + tab_data->_col_num * 100 + min(100, tabsize);
    std::vector<float> collumn(first, last);
    // table key next
    while (table_metainfo_record.table_key_next != unref)
    {
        std::string tab_key = table_metainfo_record.table_key_next;
        tabsize -= 100;
        if (tabsize < 0)
        {
            break;
        }
        table_metainfo_record = table_metainfo_cache.at(tab_key);
        index = table_metainfo_record.index + 1;
        wanda_input_file.get_float_element(group_name, element_name, {index, index, 1}, values);
        first = values.begin() + tab_data->_col_num * 100;
        last = values.begin() + tab_data->_col_num * 100 + min(100, tabsize);
        std::vector<float> temp_col(first, last);
        collumn.insert(collumn.end(), temp_col.begin(), temp_col.end());
    }
    tab_data->floattable = collumn;
}
// private method
void wanda_model::read_table(wanda_table &table)
{
    load_table();
    for (auto description : table.get_descriptions())
    {
        auto tab_data = table.get_table_data(description);
        if (tab_data->_key == unref || tab_data->_key.empty() || tab_data->_key[0] == '\0')
        {
            continue;
        }
        if (tab_data->_table_type == 'T')
        {
            std::string tables = "TABLES";
            std::string tab_vals = "Table_values";
            load_table_data(tab_data, tables, tab_vals);
        }
        else if (tab_data->_table_type == 'N')
        {
            std::string tables = "NUMCOLUMNS";
            std::string tab_vals = "Column_numval";
            load_table_data(tab_data, tables, tab_vals);
        }
        else if (tab_data->_table_type == 'S')
        {
            if (tab_data->_key != unref && !tab_data->_key.empty() && tab_data->_key[0] != '\0')
            {
                int index = table_metainfo_cache.at(tab_data->_key).index + 1;
                std::vector<std::string> values(200);
                int char_col_size = table_metainfo_cache.at(tab_data->_key).size;
                wanda_input_file.get_string_element("CHARCOLUMNS", "Column_charval", {index, index, 1}, 60, values);
                std::vector<std::string>::const_iterator first = values.begin() + tab_data->_col_num * 100;
                std::vector<std::string>::const_iterator last =
                    values.begin() + tab_data->_col_num * 100 + min(100, char_col_size);
                std::vector<std::string> collumn(first, last);
                // table key next
                while (table_metainfo_cache.at(tab_data->_key).table_key_next != unref)
                {
                    char_col_size -= 100;
                    if (char_col_size < 0)
                    {
                        break;
                    }
                    std::string tab_key = table_metainfo_cache.at(tab_data->_key).table_key_next;
                    index = table_metainfo_cache[tab_key].index + 1;
                    wanda_input_file.get_string_element("CHARCOLUMNS", "Column_charval", {index, index, 1}, 60, values);
                    first = values.begin() + tab_data->_col_num * 100;
                    last = values.begin() + tab_data->_col_num * 100 + min(100, char_col_size);
                    std::vector<std::string> temp_col(first, last);
                    collumn.insert(collumn.end(), temp_col.begin(), temp_col.end());
                }
                tab_data->stringtable = collumn;
                // table.set_string_collumn(description, collumn);
            }
        }
    }
    table.set_modified(false);
}

// private method
void wanda_model::load_message(std::string group)
{
    int size = wanda_output_file.get_maxdim_index(group);
    if (size == 0)
    {
        return;
    }
    std::vector<std::string> messages(size);
    wanda_output_file.get_string_element(group, "Message", {1, size, 1}, 256, messages);
    std::vector<std::string> message_class(size);
    wanda_output_file.get_string_element(group, "Message_class", {1, size, 1}, 1, message_class);
    std::vector<std::string> message_comp_key(size);
    wanda_output_file.get_string_element(group, "Message_comp_key", {1, size, 1}, 8, message_comp_key);
    std::vector<float> time(size);
    wanda_output_file.get_float_element(group, "Time", {1, size, 1}, time);

    for (int i = 0; i < size; i++)
    {
        if (phys_components.find(message_comp_key[i]) != phys_components.end())
        {
            phys_components[message_comp_key[i]].add_message(messages[i], message_class[i][0], time[i]);
        }
        if (ctrl_components.find(message_comp_key[i]) != ctrl_components.end())
        {
            ctrl_components[message_comp_key[i]].add_message(messages[i], message_class[i][0], time[i]);
        }
        if (phys_nodes.find(message_comp_key[i]) != phys_nodes.end())
        {
            phys_nodes[message_comp_key[i]].add_message(messages[i], message_class[i][0], time[i]);
        }
    }
}

void wanda_model::set_unit_factors()
{
    for (auto &comp : phys_components)
    {
        for (auto it = comp.second.begin(); it != comp.second.end(); ++it)
        {
            it->second.set_unit_factor(unit_list, case_units);
        }
    }
    for (auto &comp : ctrl_components)
    {
        for (auto it = comp.second.begin(); it != comp.second.end(); ++it)
        {
            it->second.set_unit_factor(unit_list, case_units);
        }
    }
    for (auto &comp : phys_nodes)
    {
        for (auto it = comp.second.begin(); it != comp.second.end(); ++it)
        {
            it->second.set_unit_factor(unit_list, case_units);
            // for (auto& item : unit_list.at(prop.get_unit_dim())) {
            //	  if (item.second.unit_factor == 1.0) {
            //	  set_unit(prop.get_unit_dim(), item.second.unit_description);
            //	  break;
            //  }
            //  }
        }
    }
}

bool wanda_model::check_name(std::string name, std::string key)
{
    if (name2_phys_comp.find(name) != name2_phys_comp.end())
    {
        return true;
    }

    for (auto &comp : ctrl_components)
    {
        if (comp.second.get_complete_name_spec() == name && comp.first != key)
        {
            return true;
        }
    }
    if (name2_phys_node.find(name) != name2_phys_node.end())
    {
        return true;
    }
    for (auto &comp : signal_lines)
    {
        if (comp.second.get_complete_name_spec() == name && comp.first != key)
        {
            return true;
        }
    }
    return false;
}

bool wanda_model::check_name(std::string name)
{
    for (auto &comp : phys_components)
    {
        if (comp.second.get_complete_name_spec() == name)
        {
            return true;
        }
    }
    for (auto &comp : ctrl_components)
    {
        if (comp.second.get_complete_name_spec() == name)
        {
            return true;
        }
    }
    for (auto &comp : phys_nodes)
    {
        if (comp.second.get_complete_name_spec() == name)
        {
            return true;
        }
    }
    for (auto &comp : signal_lines)
    {
        if (comp.second.get_complete_name_spec() == name)
        {
            return true;
        }
    }
    return false;
}

bool wanda_model::glob_var_modified()
{
    for (auto glob : global_vars)
        if (glob.second.is_modified())
            return true;
    return false;
}

std::string wanda_model::get_file_version(const std::string &executable_name)
{

#ifdef _WIN32
    DWORD dwHandle, sz = GetFileVersionInfoSizeA(executable_name.c_str(), &dwHandle);
    if (0 == sz)
    {
        throw std::runtime_error("Error in GetFileVersionInfoSizeA");
    }
    std::vector<char> buf(sz);
    // char *buf = new char[sz];
    if (!GetFileVersionInfoA(executable_name.c_str(), dwHandle, sz, buf.data()))
    {
        throw std::runtime_error("Error in GetFileVersionInfoSizeA");
    }
    VS_FIXEDFILEINFO *pvi;
    sz = sizeof(VS_FIXEDFILEINFO);
    if (!VerQueryValueA(&buf[0], "\\", (LPVOID *)&pvi, (unsigned int *)&sz))
    {
        throw std::runtime_error("Error in GetFileVersionInfoSizeA");
    }
    char *ver = new char[sz];
    sprintf(ver, "%d.%d.%d.%d", pvi->dwProductVersionMS >> 16, pvi->dwFileVersionMS & 0xFFFF,
            pvi->dwFileVersionLS >> 16, pvi->dwFileVersionLS & 0xFFFF);
    auto versionstring = std::string(ver);
    delete[] ver;
#endif
    return versionstring;
}

void wanda_model::load_lines_diagram_information()
{
    // get size of the group
    int size = wanda_input_file.get_maxdim_index("LINES");
    if (size == 0)
    {
        return;
    }
    // load information from nefis    
    std::vector<int> color(size);
    std::vector<std::string> from_key(size);
    std::vector<std::string> h_node_key(size);
    std::vector<std::string> line_key(size);
    std::vector<std::string> to_key(size);
    std::vector<int> n_coord(size);
    std::vector<std::vector<float>> x_value(size, std::vector<float>(10));
    std::vector<std::vector<float>> y_value(size, std::vector<float>(10));    
    std::vector<int> line_thickness(size);
    
    wanda_input_file.get_int_element("LINES", "Color", {1, size, 1}, color);
    wanda_input_file.get_string_element("LINES", "From_key", {1, size, 1}, 0, from_key);
    wanda_input_file.get_string_element("LINES", "H_node_key", {1, size, 1}, 0, h_node_key);
    wanda_input_file.get_string_element("LINES", "Line_key", {1, size, 1}, 0, line_key);
    wanda_input_file.get_string_element("LINES", "To_key", {1, size, 1}, 0, to_key);
    wanda_input_file.get_string_element("LINES", "From_key", {1, size, 1}, 0, from_key);
    wanda_input_file.get_int_element("LINES", "Line_thickness", {1, size, 1}, line_thickness);
    wanda_input_file.get_int_element("LINES", "N_coord", {1, size, 1}, n_coord);
    wanda_input_file.get_float_element("LINES", "X_value", {1, size, 1}, {1, 10, 1},  x_value, true);
    wanda_input_file.get_float_element("LINES", "Y_value", {1, size, 1}, {1, 10, 1}, y_value, true);

    // loop over the information and store it into the H-nodes and signal lines.
    for (int i = 0; i < size; i++)
    {
        // check if it is a h-node
        wanda_item* wanda_item_object;
        if (phys_nodes.contains(h_node_key[i]))
        {
            wanda_item_object = &phys_nodes[h_node_key[i]];
        }
        else 
        {
            // it should be a signal line
            wanda_item_object = &signal_lines[h_node_key[i]];
        }
        diagram_lines.emplace(line_key[i], wanda_diagram_lines(line_key[i], wanda_item_object, from_key[i], to_key[i],
                                                  n_coord[i], x_value[i], y_value[i], color[i], line_thickness[i]));
        wanda_item_object->add_lines_info(&diagram_lines[line_key[i]]);
    }
}

void wanda_model::save_lines_diagram_information()
{
    std::vector<int> color;
    std::vector<std::string> from_key;
    std::vector<std::string> h_node_key;
    std::vector<std::string> line_key;
    std::vector<std::string> to_key;
    std::vector<std::vector<float>> x_value;
    std::vector<std::vector<float>> y_value;
    std::vector<int> n_coord;
    std::vector<int> line_thickness;

    for (auto &diag_line : diagram_lines)
    {
        color.push_back(diag_line.second.get_color());
        from_key.push_back(diag_line.second.get_from_key());
        h_node_key.push_back(diag_line.second.get_node_key()->get_key_as_string());
        line_key.push_back(diag_line.second.get_key_as_string());
        to_key.push_back(diag_line.second.get_to_key());
        auto x = diag_line.second.get_x_value();
        auto y = diag_line.second.get_y_value();
        x_value.push_back(x);
        y_value.push_back(y);
        n_coord.push_back((x_value.end() - 1)->size());
        line_thickness.push_back(diag_line.second.get_line_thickness());
    }

    // save everything to wdi
    int size = static_cast<int>(color.size());
    if (size == 0)
    {
        return;
    }
    int x_size = static_cast<int>( x_value[0].size());
    if (x_size > 10)
    {
        throw std::runtime_error("Line has to many x or y values");
    }
    //TODO how to cope with when there are only two values in the x and y
    for (int i = x_size; i<10; i++)
    {
        //x_value.push_back(0.0);
       // y_value.push_back(0.0);
    }

    wanda_input_file.write_int_elements("LINES", "Color", {1, size, 1}, color);
    wanda_input_file.write_string_elements("LINES", "From_key", {1, size, 1}, 0, from_key);
    wanda_input_file.write_string_elements("LINES", "H_node_key", {1, size, 1}, 0, h_node_key);
    wanda_input_file.write_string_elements("LINES", "Line_key", {1, size, 1}, 0, line_key);
    wanda_input_file.write_string_elements("LINES", "To_key", {1, size, 1}, 0, to_key);
    wanda_input_file.write_string_elements("LINES", "From_key", {1, size, 1}, 0, from_key);
    wanda_input_file.write_int_elements("LINES", "Line_thickness", {1, size, 1}, line_thickness);
    wanda_input_file.write_int_elements("LINES", "N_coord", {1, size, 1}, n_coord);
    wanda_input_file.write_float_elements("LINES", "X_value", {1, size, 1}, {1, x_size, 1}, x_value);
    wanda_input_file.write_float_elements("LINES", "Y_value", {1, size, 1}, {1, x_size, 1}, y_value);
}

void wanda_model::load_diagram_text_boxes()
{
    int size = wanda_input_file.get_maxdim_index("DIAGRAM");
    if (size == 0)
    {
        return;
    }
    std::vector<int> background_color(size);
    std::vector<int> color(size);
    std::vector<int> line_thickness(size);
    std::vector<int> font_size(size);
    std::vector top_left(size, std::vector<float>(2));
    std::vector<float> height(size);
    std::vector<float> width(size);
    std::vector<std::string> font(size);
    std::vector<std::string> text(size);
    std::vector<std::string> text_position(size);

    wanda_input_file.get_int_element("DIAGRAM", "Backgr_color", {1, size, 1}, background_color);
    wanda_input_file.get_int_element("DIAGRAM", "Color", {1, size, 1}, color);
    wanda_input_file.get_int_element("DIAGRAM", "Line_thickness", {1, size, 1}, line_thickness);
    wanda_input_file.get_int_element("DIAGRAM", "Font_size", {1, size, 1}, font_size);
    wanda_input_file.get_float_element("DIAGRAM", "Height", {1, size, 1}, height);
    wanda_input_file.get_float_element("DIAGRAM", "Width", {1, size, 1}, width);
    wanda_input_file.get_float_element("DIAGRAM", "Top_left", {1, size, 1}, {1, 2, 1}, top_left, true);
    wanda_input_file.get_string_element("DIAGRAM", "Font", {1, size, 1}, 0, font);
    wanda_input_file.get_string_element("DIAGRAM", "Text", {1, size, 1}, 0, text);
    wanda_input_file.get_string_element("DIAGRAM", "Text_position", {1, size, 1}, 0, text_position);


    for (int i = 0; i < size; i++)
    {
        auto key = get_unique_key(&diagram_text_boxes, 'D', last_key);        
        diagram_text_boxes.insert(
            std::pair(
            key, diagram_text(key, text_field(color[i], font_size[i], font[i], text_position[i], text[i]),
                              background_color[i],
                          line_thickness[i], coordinates(top_left[i]), width[i], height[i])));        
    }
}

void wanda_model::save_diagram_text_boxes()
{
    int const size = static_cast<int>(diagram_text_boxes.size());
    if (size == 0)
    {
        return;
    }   
    std::vector<int> background_color;
    std::vector<int> color;
    std::vector<int> line_thickness;
    std::vector<int> font_size;
    std::vector<float> height;
    std::vector<float> width;
    std::vector<std::vector<float>> top_left;
    std::vector<std::string> font;
    std::vector<std::string> text;
    std::vector<std::string> text_position;
    for (auto& text_box: diagram_text_boxes)
    {
        background_color.emplace_back(text_box.second.background_color);
        color.emplace_back(text_box.second.text.color);
        line_thickness.emplace_back(text_box.second.line_thickness);
        font_size.emplace_back(text_box.second.text.font_size);
        height.emplace_back(text_box.second.height);
        font.emplace_back(text_box.second.text.font);
        width.emplace_back(text_box.second.width);
        text.emplace_back(text_box.second.text.text);
        text_position.emplace_back(text_box.second.text.get_text_position());
        top_left.emplace_back(text_box.second.top_left.get_coordinates());
    }

    wanda_input_file.write_int_elements("DIAGRAM", "Backgr_color", {1, size, 1}, background_color);
    wanda_input_file.write_int_elements("DIAGRAM", "Color", {1, size, 1}, color);
    wanda_input_file.write_int_elements("DIAGRAM", "Line_thickness", {1, size, 1}, line_thickness);
    wanda_input_file.write_int_elements("DIAGRAM", "Font_size", {1, size, 1}, font_size);
    wanda_input_file.write_float_elements("DIAGRAM", "Height", {1, size, 1}, height);
    wanda_input_file.write_float_elements("DIAGRAM", "Width", {1, size, 1}, width);
    wanda_input_file.write_float_elements("DIAGRAM", "Top_left", {1, size, 1}, {1, 2, 1}, top_left);
    wanda_input_file.write_string_elements("DIAGRAM", "Font", {1, size, 1}, 0, font);
    wanda_input_file.write_string_elements("DIAGRAM", "Text", {1, size, 1}, 0, text);
    wanda_input_file.write_string_elements("DIAGRAM", "Text_position", {1, size, 1}, 0, text_position);
}


void wanda_model::delete_text_box(const std::string &key)
{
    if (diagram_text_boxes.find(key) == diagram_text_boxes.end())
    {
        throw std::invalid_argument("No text box with key: " + key + " exists in model");
    }
    diagram_text_boxes.erase(key);
}

std::vector<std::string> wanda_model::get_all_diagram_lines()
{
    std::vector<std::string> lines;
    for (auto diagram_line : diagram_lines)
    {
        lines.emplace_back(diagram_line.first);
    }
    return lines;
}

std::vector<diagram_text> wanda_model::get_all_text_boxes()
{
    std::vector<diagram_text> diagram_texts;
    for (auto &text_box : diagram_text_boxes)
    {
        diagram_texts.emplace_back(text_box.second);
    }
    return diagram_texts;
}

 wanda_diagram_lines &wanda_model::get_diagram_line(const std::string &diagram_line_key)
{
     if (diagram_lines.contains(diagram_line_key))
     {
         return diagram_lines[diagram_line_key];
     }
     throw std::invalid_argument("No line with key: " + diagram_line_key + " exists in model");     
 }

void wanda_model::delete_diagram_line(const std::string &key)
{
    if (diagram_lines.contains(key))
    {
        diagram_lines.erase(key);
        return;
    }
    throw std::invalid_argument("No line with key: " + key + " exists in model");
}
