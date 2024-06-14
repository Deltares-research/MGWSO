#ifndef _WANDAPROP_
#define _WANDAPROP_

#include <vector>
#include <wanda_table.h>

#ifdef WANDAMODEL_EXPORT
// #define WANDAMODEL_API __declspec(dllexport)
#define WANDAMODEL_API 
#else
#define WANDAMODEL_API __declspec(dllimport)
#endif
///@private
struct wanda_prop_template
{
    std::string value = "";
    std::vector<std::vector<std::string>> table{};
};

inline std::vector<std::vector<std::string>> transpose(std::vector<std::vector<std::string>> in)
{
    std::vector<std::vector<std::string>> outtrans(in[0].size(), std::vector<std::string>(in.size()));
    for (size_t i = 0; i < in.size(); ++i)
        for (size_t j = 0; j < in[i].size(); ++j)
            outtrans[j][i] = in[i][j];
    return outtrans;
}

enum class wanda_property_types
{
    GLOQUANT,
    HIS,
    HCS,
    HOS,
    HOV,
    CIS,
    COS,
    COV,
    NIS,
    NOS,
    NOV,
    GLOV,
    MAO,
    NONE
};

//!  main class for the Wanda properties.
/*!
The Wanda property class is used to access properties of components (e.g.
Pressure). It can be used to changes input properties and to request output
time series. A property can be requested from a wanda component or the wanda
model for general properties
*/
class WANDAMODEL_API wanda_property
{
  public:
    ///@private
    wanda_property();
    ///@private
    wanda_property(int index, std::string spec_descr, char comp_spec_code, char comp_sp_inp_fld,
                   std::string wdo_post_fix, std::string unit_dim, wanda_property_types wnd_type,
                   std::string short_quant_name, float def_val = 0, float min_val = 0, float max_val = 0,
                   std::string list_dependency = "", int view_list_mask = 0, char input_type_code = ' ',
                   int view_mask = 0);
    //! returns the name of the property
    std::string get_description() const
    {
        return _description;
    }
    ///@private
    void set_description(std::string description)
    {
        _description = description;
    }
    ///@private
    std::string get_wdo_postfix() const
    {
        return _wdo_post_fix;
    }
    ///@private
    void set_wdo_postfix(std::string wdo_post_fix)
    {
        _wdo_post_fix = wdo_post_fix;
    }
    ///@private
    int get_index() const
    {
        return _index;
    }
    ///@private
    void set_index(int index)
    {
        _index = index;
    }
    ///@private
    int get_group_index() const
    {
        return _group_index;
    }
    ///@private
    void set_group_index(int group_index)
    {
        _group_index = group_index;
    }
    ///@private
    int get_hos_index() const
    {
        return _hos_index;
    }
    ///@private
    void set_hos_index(int hos_index)
    {
        _hos_index = hos_index;
    }
    ///@private
    char get_property_spec_inp_fld() const
    {
        return _comp_sp_inp_fld;
    }
    ///@private
    void set_property_spec_inp_fld(char inp_fld)
    {
        _comp_sp_inp_fld = inp_fld;
    }
    ///@private
    char get_property_spec_code() const
    {
        return _comp_spec_code;
    }
    //! Returns the multiplication factor from SI to the unit of the wanda model for this property.
    float get_unit_factor() const
    {
        return _unit_fac;
    }
    ///@private
    void set_unit_factor(float factor)
    {
        _unit_fac = factor;
    }
    ///@private
    void set_unit_factor(std::unordered_map<std::string, std::unordered_map<std::string, float>> &unit_list,
                         std::unordered_map<std::string, std::string> &case_units);
    //! returns the unit dimension of the property
    std::string get_unit_dim() const
    {
        return _unit_dim;
    }
    ///@private
    void set_unit_dim(std::string unit_dim)
    {
        _unit_dim = unit_dim;
    }
    ///@private
    void set_extremes(float *, float *, float *, float *);
    ///@private
    void set_extremes(int, float *, float *, float *, float *);
    //! Returns the minimum value of the time series of the component
    float get_extr_min() const;
    //! Returns the maximum value of the time series of the component
    float get_extr_max() const;
    //! Returns the time at which the minimum value of the time series of the
    //! component occurs
    float get_extr_tmin() const;
    //! Returns the time at which the maximum value of the time series of the
    //! component occurs
    float get_extr_tmax() const;
    //! Returns the minimum value of the time series of the component at the given element number
    /*!
     \param element, element in component for which the minimum value is returned
     */
    float get_extr_min(int element) const;
    //! Returns the maximum value of the time series of the component at the given element number
    /*!
    \param element in component for which the maximum value is returned
    */
    float get_extr_max(int element) const;
    //! Returns the time at which the  minimum value occurs
    /*!
    \param element in component for which the minimum value is returned
    */
    float get_extr_tmin(int element) const;
    //! Returns the time at which the  maximum value occurs
    /*!
    \param element in component for which the maximum value is returned
    */
    float get_extr_tmax(int element) const;
    //! Returns vector of minimum values in the pipe
    std::vector<float> get_extr_min_pipe() const;
    //! Returns vector of maximum values in the pipe
    std::vector<float> get_extr_max_pipe() const;
    //! Returns vector of times at which the minimum values in the pipe occurs
    std::vector<float> get_extr_tmin_pipe() const;
    //! Returns vector of times at which the maximum values in the pipe occurs
    std::vector<float> get_extr_tmax_pipe() const;
    ///@private
    void set_number_of_elements(int i);
    //! returns the value of the property.
    //! This can be either the input value, or the
    //! output value or the value at the first time step depending on the type of property.
    float get_scalar_float() const;
    //! returns the selected value for a drop down list
    std::string get_scalar_str();
    //! Sets the input value to the given value
    /*!
     \param scalar value to change the input property to
     */
    void set_scalar(float scalar);
    //! Sets the input value to the given value
    /*!
    \param scalar reference to value to change the input property to
    */
    void set_scalar_by_ref(float &scalar);
    //! Sets a drop down list to the given string
    /*!
    \param scalar string to change the drop down list to
    */
    void set_scalar(std::string scalar);
    ///@private
    void set_series_by_ref(std::vector<float> *series);
    ///@private
    void set_series_pipe_by_ref(int element, std::vector<float> *series);
    //! returns time series of the property
    std::vector<float> get_series() const;
    //! returns time series of the property at the given element
    /*!
     \param element of the pipe for which the time series is returned
     */
    std::vector<float> get_series(int element) const;
    //! Returns time series of the property at all elements in a pipe.
    std::vector<std::vector<float>> get_series_pipe() const;
    //! Returns true when the property is an input or an output value
    bool has_scalar() const;
    //! Returns true when the property has a table
    bool has_table() const;
    //! Returns true when the property has a time series
    bool has_series() const;
    ///@private
    bool is_glo_quant() const;
    ///@private
    void settype();
    ///@private
    wanda_property_types get_property_type() const
    {
        return _wnd_type;
    }
    //! returns for a drop downlist the item at the given number
    /*!
     \param num_item is the number in list for which to return the item. Count
     starts at 1.
     */
    std::string get_list_item(int num_item);
    ///@private
    void set_list(std::vector<std::string> list);
    //! Returns entire input list for a drop down list.
    std::vector<std::string> get_list() const;
    //! Returns the selected item of a drop down list
    std::string get_selected_item();
    ///@private
    bool is_modified() const;
    ///@private
    void set_modified(bool status);
    //! Returns a wanda table item, when the property has a table.
    wanda_table &get_table();
    //! Returns the number of elements of the property
    int get_number_of_elements() const
    {
        return _number_of_elements;
    }
    //! Returns the minimum possible input value
    float get_min_input_value() const
    {
        return _min_value;
    }
    //! Returns the maximum recommended input value
    float get_max_input_value() const
    {
        return _max_value;
    }
    //! Returns the default input value
    float get_default_input_value() const
    {
        return _default_value;
    }
    ///@private
    bool get_spec_status() const
    {
        return _spec_status;
    }
    ///@private
    void set_spec_status(bool spec_stat)
    {
        _spec_status = spec_stat;
    }
    //! return true when the property is an input
    bool is_input() const
    {
        return _wnd_type == wanda_property_types::HIS || _wnd_type == wanda_property_types::NIS ||
               _wnd_type == wanda_property_types::CIS;
    }
    //! return true when the property is an input
    bool is_output() const
    {
        return _wnd_type == wanda_property_types::HOS || _wnd_type == wanda_property_types::NOS ||
               _wnd_type == wanda_property_types::COS;
    }
    ///@private
    std::string get_short_quant_name() const
    {
        return _short_quant_name;
    }
    //! returns true when the value is a string or a drop down list
    bool has_string() const
    {
        return _comp_sp_inp_fld == 'C';
    }
    //! sets the value of the property to the value from the property template
    void set_value_from_template(std::unordered_map<std::string, wanda_prop_template>::value_type item);
    ///@private
    void set_con_point_quant(bool val)
    {
        _con_point_quant = val;
    }
    ///@private
    bool is_con_point_quant() const
    {
        return _con_point_quant;
    }
    ///@private
    std::string get_list_dependency() const
    {
        return _list_dependency;
    }
    ///@private
    std::vector<int> get_view_list_numbers() const;
    ///@private
    char get_input_type_code() const
    {
        return _input_type_code;
    }
    ///@private
    int get_view_mask() const
    {
        return _view_mask;
    }
    ///@private
    // TODO make disused of the property a pointer to the disused of the component so that you do not have to change all
    // the discussed settings for the properties when changing disused of the component.
    void set_disused(bool status)
    {
        disused = status;
    }
    bool is_disused() const
    {
        return disused;
    }
    //! returns hash of the object string for object identification (used in c_wanda_engine)
    std::size_t wnd_get_hash()
    {
        return _object_hash;
    }
    ///@private
    void set_species_number(int number)
    {
        species_number = number;
    }
    ///@private
    int get_species_number() const
    {
        return species_number;
    }
    ///@private
    void set_connection_point(int con_point)
    {
        connection_point = con_point;
    }
    ///@private
    int get_connection_point() const
    {
        return connection_point;
    }
    // copies data from one input to the other
    void copy_data(wanda_property prop_org);

  private:
    static std::string _object_name;
    std::size_t _object_hash = std::hash<std::string>{}(_object_name);
    bool _modified = false;
    float _scalar = -999.0f;
    std::vector<float> *_series;                    // HOS
    std::vector<std::vector<float> *> _series_pipe; // HOM/gloq for pipes
    std::vector<float *> extr_min;
    std::vector<float *> extr_max;
    std::vector<float *> extr_tmin;
    std::vector<float *> extr_tmax;
    std::vector<std::string> drop_down_list;
    wanda_table _table;
    wanda_property_types _wnd_type;
    std::string _description;
    int _index;
    int _group_index;
    char _comp_spec_code;      // Indicates where the property is stored, common or
                               // oper specs (only used in physical components)
    char _comp_sp_inp_fld;     // type of property, real, char, table, etc.
    std::string _wdo_post_fix; // Postfix for group name in Nefis file
    std::string _unit_dim;
    float _unit_fac = 0;
    int _hos_index = 0;
    float _default_value = 0;
    float _min_value = -1e8f;
    float _max_value = 1e8f;
    int _number_of_elements = 0;
    ;
    bool _spec_status = false;
    std::string _short_quant_name;
    bool _con_point_quant = true;
    std::string _list_dependency;
    int _view_list_mask = 0;
    char _input_type_code = ' ';
    int _view_mask = 0;
    bool disused = false;
    int species_number = 0;
    int connection_point = 0; // Shows to which connection point this quantity belongs, if it is not belong to a
                              // connection point it is zero
};

#endif
