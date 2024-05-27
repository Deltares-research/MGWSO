#ifndef _WANDADEF_
#define _WANDADEF_

#include <nefis_file.h>
#include <unordered_map>
#include <unordered_set>
#include <wandacomponent.h>
#include <wandanode.h>
#include <wandaproperty.h>
#include <deltares_helper_functions.h>

struct wanda_tab_info
{
    std::vector<std::string> description;
    std::vector<std::string> unit_dim;
    std::vector<float> min_val;
    std::vector<float> max_val;
    std::vector<int> min_points_req;
    std::vector<int> max_points_req;
};

class wanda_def
{
  public:
    //    wanda_def();
    wanda_def(std::string data_path);
    ~wanda_def();
    wanda_component *get_component(std::string classsort_key);
    wanda_node *get_node(std::string classsort_key);
    std::unordered_map<std::string, wanda_property> get_properties(std::string classsort_key);
    std::unordered_map<std::string, wanda_property> get_physical_input_properties(std::string classsort_key);
    std::unordered_map<std::string, wanda_property> get_physical_calc_properties(std::string classsort_key);
    std::unordered_map<std::string, wanda_property> get_physical_output_properties(std::string classsort_key);
    std::unordered_map<std::string, wanda_property> get_control_input_properties(std::string classsort_key);
    std::unordered_map<std::string, wanda_property> get_control_output_properties(std::string classsort_key);
    std::unordered_map<std::string, wanda_property> get_node_input_properties(std::string classsort_key);
    std::unordered_map<std::string, wanda_property> get_node_output_properties(std::string classsort_key);
    std::string get_name_prefix_phys_comp(std::string classsort_key);
    std::string get_name_prefix_phys_node(std::string classsort_key);
    std::string get_name_prefix_ctrl_comp(std::string classsort_key);
    std::string get_class_name_phys_comp(std::string classsort_key);
    std::string get_class_name_phys_node(std::string classsort_key);
    std::string get_class_name_ctrl_comp(std::string classsort_key);
    std::string get_type_phys_comp(std::string classsort_key);
    int get_physical_num_com_his(std::string classsort_key);
    int get_physical_num_ope_his(std::string classsort_key);
    bool is_physical_component(std::string classsort_key);
    bool is_control_component(std::string classsort_key);
    bool is_node(std::string classsort_key);
    std::unordered_map<std::string, std::unordered_map<std::string, float>> get_unit_list();
    std::unordered_map<std::string, std::string> get_case_unit(std::string unit_group);
    std::string get_class_sort_key(std::string type);
    std::vector<std::string> get_core_quants(std::string classsort_key);
    std::string get_node_core_quants(std::string classsort_key);
    std::string get_node_type(std::string classsort_key);
    std::vector<int> get_num_input_props(std::string classsort_key);
    void get_ip_fld_his(std::string classsort_key, std::vector<std::string> &com, std::vector<std::string> &ope);
    void get_ip_fld_his(std::string classsort_key, std::vector<std::string> &com);
    std::vector<std::string> get_ctrl_in_type(std::string classsort_key);
    std::vector<std::string> get_ctrl_out_type(std::string classsort_key);
    std::vector<int> get_num_input_chanl(std::string classsort_key);
    std::vector<int> get_num_output_chanl(std::string classsort_key);
    std::vector<std::string> get_comp_spec_ip_fld(std::string classsort_key);
    std::vector<int> get_max_in_chan(std::string classsort_key);
    std::vector<int> get_min_in_chan(std::string classsort_key);
    std::vector<std::string> get_in_chan_type(std::string classsort_key);
    std::vector<std::string> get_out_chan_type(std::string classsort_key);
    std::string get_ctrl_input_type(std::string classsort_key);
    std::string get_data_path() const;
    int get_num_con_points(std::string classsort_key);
    std::string get_quant_name(char symbol);
    char get_quant_symbol(std::string quant_name);
    std::vector<std::string> get_list_quant_names(std::string classsort_key);
    int get_number_of_con_points(std::string classsort_key);
    bool get_controlable(std::string classsort_key);
    float get_unit_factor(std::string unit_dim, std::string unit_descr);
    std::string get_type_name_phys(std::string classsort_key);
    std::string get_type_name_node(std::string classsort_key);
    int get_max_num_species() const
    {
        return max_num_species;
    }
    bool check_def_mask(std::string class_sort_key, char mask);
    bool is_obsolete(std::string class_sort_key);
    bool is_prototype(std::string class_sort_key);
    bool is_special(std::string class_sort_key);
    std::vector<std::string> get_possible_phys_comp_type();
    std::vector<std::string> get_possible_ctrl_comp_type();
    std::vector<std::string> get_possible_node_type();
    int get_element_size(std::string &element);
    std::string get_wanda_version() const;
  private:
    void initialize(std::string data_path);
    bool initialized = false;
    nefis_file _database;
    int _num_phys_comp;
    int _num_ctrl_comp;
    std::unordered_set<std::string> _phys_class_names; // used for checking if component is a physical
                                                       // component
    std::unordered_set<std::string> _phys_class_sort_keys;
    std::unordered_set<std::string> _node_class_sort_keys;
    std::unordered_set<std::string> _control_class_sort_keys;
    std::unordered_map<std::string, std::string> _phys_type_2_classsort;
    std::unordered_map<std::string, std::string> _node_type_2_classsort;
    std::unordered_map<std::string, std::string> _ctrl_type_2_classsort;
    std::unordered_map<std::string, std::unordered_map<std::string, wanda_property>> phys_input_props;
    std::unordered_map<std::string, std::unordered_map<std::string, wanda_property>> phys_output_props;
    std::unordered_map<std::string, std::unordered_map<std::string, wanda_property>> phys_props;
    std::unordered_map<std::string, std::unordered_map<std::string, wanda_property>> node_props;
    std::unordered_map<std::string, wanda_component> _components_list;
    std::unordered_map<std::string, wanda_node> _node_list;
    std::unordered_map<std::string, wanda_tab_info> table_info;
    std::unordered_map<std::string, wanda_tab_info> num_col_info;
    std::unordered_map<std::string, wanda_tab_info> string_col_info;
    std::unordered_map<std::string, std::string> ctrl_input_type;
    std::string _data_path;
    void load_class_sort_keys();
    void load_table_types();
    void load_numcol_types();
    void load_string_col_types();
    void load_unit_list();
    std::string to_up(std::string) const;
    static int get_key_index_array(std::vector<std::string> kay_array, std::string key);
    std::unordered_map<std::string, std::unordered_map<std::string, float>> unit_list;
    std::string get_type_name_ctrl(std::string classsort_key);
    std::string get_convert2_comp(std::string classsort_key);
    std::string get_default_mask(std::string classsort_key);
    int max_num_species = 20;
    void load_version_number();
    wanda_helper_functions::wanda_version_number wanda_version_ = {"0.0.0"};
};
#endif
