#ifndef _WANDACOMP_
#define _WANDACOMP_
#include <map>
#include <unordered_map>
#include <vector>
#include <wanda_item.h>

#ifdef WANDAMODEL_EXPORT
// #define WANDAMODEL_API __declspec(dllexport)
#define WANDAMODEL_API 
#else
#define WANDAMODEL_API __declspec(dllimport)
#endif

class wanda_def;
class wanda_node;
class wanda_sig_line;

struct table_data
{
    std::vector<float*> column1;
    std::vector<float*> column2;
    std::vector<int> size_tables;
    int _size;

    table_data(int size) : _size(size), column1(size), column2(size), size_tables(size)
    {
    }
    table_data() = default;

};

//!  main class for the Wanda component.
/*!
The wanda_component class is the main interface for wanda components. This class
gives the user access to all the properties of a components. This class is a
derivated class of the wanda_item class
*/
class WANDAMODEL_API wanda_component : public wanda_item
{
  public:
    ///@private
    wanda_component();
    ///@private
    wanda_component(int compkey, std::string classname, std::string csKey, std::string namePrefix, std::string name,
                    wanda_type type, std::string physcomp_type, std::string type_name, wanda_def *component_definition);
    ///@private
    wanda_component(std::string class_sort_key, std::string name_pre_fix, wanda_type type, std::string type_comp,
                    int num_com_spec, int num_oper_spec, int num_hcs, int number_of_connnect_points, bool controlable,
                    std::vector<std::string> core_quants, std::unordered_map<std::string, wanda_property> props,
                    std::string h_ctrl_input, std::string type_name, std::string def_mask, std::string conv2comp,
                    wanda_def *component_definition);
    ///@private
    void set_num_elements(int num_elements);
    //! Returns the number of elements in a component
    int get_num_elements() const
    {
        return _num_elements;
    }
    //! Returns the number of common input specs in a component
    int get_num_common_specs() const
    {
        return _num_common_specs;
    }
    //! Returns the number of operational input specs in a component
    int get_num_oper_specs() const
    {
        return _num_oper_specs;
    }
    //! Returns the number of hydraulic calculated specs
    int get_num_hcs() const
    {
        return _num_hcs;
    }
    //! Returns the component type
    std::string get_physcomp_type() const
    {
        return _physcomp_type;
    }
    //! sets the component type
    /*!
     \param type is physical component type
     */
    void set_physcomp_type(std::string type)
    {
        _physcomp_type = type;
    }
    //! Sets the angle of the component in the Igrafix diagram.
    /*!
     *set_angle() sets the angle of the component as displayed in the Igrafix
     diagram. *rotation is counter clockwise \param angle of component in grfaix
     diagram in degree
     */
    void set_angle(float angle)
    {
        _shape_angle = angle;
    }
    //! Sets the IsFlipped status of the component in the Igrafix diagram.
    /*!
    *set_flipped() sets the IsFlipped status of the component as displayed in the
    Igrafix diagram. \param status if IsFlipped flag
    */
    void set_flipped(bool isFlipped);
    //! returns angle in degrees of te component in the Igrafix diagram
    float get_angle() const;
    //! returns angle in radians of te component in the Igrafix diagram
    float get_angle_rad() const;
    //! Gets the IsFlipped status of the component in the Igrafix diagram.
    bool get_is_flipped() const;
    ///@private
    std::string get_core_quants(int) const;
    ///@private
    void set_comp_num(int comp_num)
    {
        _comp_num = comp_num;
    }
    ///@private
    int get_comp_num() const
    {
        return _comp_num;
    }
    ///@private
    void set_max_input_channels(std::vector<int> max_in_ch)
    {
        max_input_channels = max_in_ch;
    }
    ///@private
    void set_min_input_channels(std::vector<int> min_in_ch)
    {
        min_input_channels = min_in_ch;
    }
    ///@private
    void set_input_channel_type(std::vector<std::string> max_in_ch);
    std::string get_input_channel_type(int connection_point) const;
    ///@private
    void set_output_channel_type(std::vector<std::string> max_in_ch);
    std::string get_output_channel_type(int connection_point) const;
    /// @private
    void fill_sensor_list(wanda_component comp, int con_point);
    /// @private
    void fill_sensor_list(wanda_node const node);
    /// @private
    void connect(wanda_node &node, int connection_point);
    /// @private
    void connect(wanda_sig_line &sig_line, int connection_point, bool is_input);
    /// @private
    void disconnect(int connection_point);
    /// @private
    void disconnect(wanda_node &node);
    /// @private
    void disconnect(wanda_sig_line &node);
    ///@private
    void disconnect(int connection_point, bool input);
    //! Returns true when a node is connected to the given connection point
    //! otherwise false is returned
    /*!
     \param connection_point of the component to check if it is connected
     */
    bool is_node_connected(const int connection_point)
    {
        return _connected_nodes.find(connection_point) != _connected_nodes.end()
                   ? true
                   : false; // TODO can the true and false not be removed?
    }
    //! Returns true when a signal line is connected to the given connection point
    //! otherwise false is returned
    /*!
    \param connection_point, connection point of the component to check if it is
    connected \param is_input, true if the input side should be checked and false
    if the output side should be checked
    */
    bool is_sigline_connected(const int connection_point, bool is_input);
    //! returns a reference to the node connected to the connection point
    /*!
    \param connection_point, connection point for which the connected node is
    returned
    */
    wanda_node &get_connected_node(const int connection_point);
    //! returns a reference to the node connected to the connection point
    /*!
    \param connection_point, connection point for which the connected node is
    returned
    */
    std::vector<wanda_node *> get_connected_nodes();

    //! returns a reference to the signal line connected to the connection point
    /*!
    \param connection_point, connection point for which the connected node is
    returned \param is_input, true when the input should be checked and false when
    the output should be checked.
    */
    std::vector<wanda_sig_line *> get_connected_sigline(int connection_point, bool is_input);
    //! returns connection point number to which the given node is connected
    /*!
    \param node, node for which to check to which connect point it is connected
    */
    int get_connect_point(wanda_item &node);
    //! Returns true when the component is a pipe
    bool is_pipe() const;
    ///@private
    void fill_profile_table();

    ///@private
    void set_ctrl_input_type(std::string ctrl_input_type)
    {
        _ctrl_input_type = ctrl_input_type;
    }
    //! returns the control input type
    std::string get_ctrl_input_type() const
    {
        return _ctrl_input_type;
    }
    //! Returns the number of connection points of the component
    int get_number_of_connnect_points() const
    {
        return _number_of_connnect_points;
    }
    //! returns true if the component can be controlled
    bool is_controlable() const
    {
        return _is_controlable;
    }

    //! Sets the material name to the string provided by the user
    /*!
    *set_material_name() changes the material name of the component to the one
    provided by the user. *Only pipes have a material name field and the maximum
    length is 24 \param material, material name string to set in the component
    */
    void set_material_name(std::string material) override;
    ///@private
    int get_num_input_channels() const
    {
        return num_input_channels;
    }
    ///@private
    int get_num_output_channels() const
    {
        return num_output_channels;
    }
    //! Returns the minimum number of output channels for the given input connect point.
    int get_min_input_channels(int con_point) const;
    //! Returns the maximum number of output channels for the given input connect point.
    int get_max_input_channels(int con_point) const;
    //! changes the geometry setting to the given choice, also copies the information to the new choice
    void change_profile_tab(std::string choice, std::vector<float> globvars);
    //! returns the area of the component if it has an area.
    float get_area();
    //! returns a vector with all the input values
    std::vector<float> get_his_values() const;
    std::vector<float> get_height_nodes();
    table_data get_table_data();
    int get_number_hsc() const;
    void set_hsc_results(std::vector<float> results);
    void set_length_from_hsc(std::vector<float> his);
    void set_hcs_error(std::string error)
    {
        hcs_error = error;
    }
    std::string get_hcs_error()
    {
        return hcs_error;
    }
    bool has_hcs_error()
    {
        return !hcs_error.empty();
    }
    std::size_t wnd_get_hash()
    {
        return _object_hash;
    }

  private:
    const static std::string _object_name;
    const std::size_t _object_hash;
    void initialize();
    std::string _physcomp_type;
    int _num_common_specs = 0;
    int _num_oper_specs = 0;
    int _num_hcs = 0;
    float _shape_angle = 0;
    int _num_elements = 0;
    std::vector<int> max_input_channels;
    std::vector<int> min_input_channels;
    std::vector<std::string> input_chan_type;
    std::vector<std::string> output_chan_type;
    std::unordered_map<int, wanda_node *> _connected_nodes;
    std::multimap<int, wanda_sig_line *> _connected_siglines_input;
    std::multimap<int, wanda_sig_line *> _connected_siglines_output;
    std::string _ctrl_input_type;
    int _number_of_connnect_points = 1;
    bool _is_controlable = false;
    int _comp_num = -999;
    int num_input_channels = 0;
    int num_output_channels = 0;
    bool _is_flipped = false;
    wanda_def *_component_definition = nullptr;
    std::string hcs_error = "";
};
#endif
