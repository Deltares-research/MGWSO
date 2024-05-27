#include <calc_hcs.h>
#include <map>
#include <numbers>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <wanda_item.h>
#include <wandacomponent.h>
#include <wandadef.h>
#include <wandanode.h>
#include <wandasigline.h>

const std::string wanda_component::_object_name = "WandaComponent Object";

wanda_component::wanda_component()
    : wanda_item(0, "empty", "empty", "empty", "empty", wanda_type::physical, ""), _physcomp_type("NONE"),
      _num_elements(0), _number_of_connnect_points(1), _object_hash(std::hash<std::string>{}(_object_name))
{
    throw std::runtime_error("Invalid call to default constructor of wanda_component");
}

wanda_component::wanda_component(int compkey, std::string classname, std::string cskey, std::string nameprefix,
                                 std::string name, wanda_type type, std::string physcomp_type, std::string type_name,
                                 wanda_def *component_definition)
    : wanda_item(compkey, classname, cskey, nameprefix, name, type, type_name), _physcomp_type(physcomp_type),
      _num_elements(0), _number_of_connnect_points(1), _component_definition(component_definition),
      _object_hash(std::hash<std::string>{}(_object_name))
{
    initialize();
}

wanda_component::wanda_component(std::string class_sort_key, std::string name_pre_fix, wanda_type type,
                                 std::string physcomp_type, int num_com_spec, int num_oper_spec, int num_hcs,
                                 int number_of_connnect_points, bool controlable, std::vector<std::string> core_quants,
                                 std::unordered_map<std::string, wanda_property> props, std::string h_ctrl_input,
                                 std::string type_name, std::string def_mask, std::string convert2comp,
                                 wanda_def *component_definition)
    : wanda_item(class_sort_key, name_pre_fix, type, def_mask, convert2comp, type_name), _physcomp_type(physcomp_type),
      _num_common_specs(num_com_spec), _num_oper_specs(num_oper_spec), _num_hcs(num_hcs),
      _number_of_connnect_points(number_of_connnect_points), _ctrl_input_type(h_ctrl_input),
      _is_controlable(controlable), _component_definition(component_definition),
      _object_hash(std::hash<std::string>{}(_object_name))
{
    _core_quantities = core_quants;
    properties = props;
    initialize();
}

void wanda_component::set_num_elements(const int num_elements)
{
    _num_elements = num_elements;
    for (auto &prop : properties)
    {
        if (prop.second.is_glo_quant())
        {
            // std::string temp =
            // (prop.second.get_description()).substr((prop.second.get_description()).size()
            // - 1, 1); int con = strtol(temp.c_str(), nullptr, 10);
            if (!prop.second.is_con_point_quant())
            {
                prop.second.set_number_of_elements(_num_elements);
            }
        }
    }
}

bool wanda_component::is_pipe() const
{
    if (_physcomp_type == "PIPE")
    {
        return true;
    }
    return false;
}

int wanda_component::get_min_input_channels(int con_point) const
{
    if (con_point <= num_input_channels)
    {
        return min_input_channels[con_point - 1];
    }
    throw std::invalid_argument(get_complete_name_spec() +
                                " - Invalid connection point number: " + std::to_string(con_point) +
                                ". Number of inputs: " + std::to_string(num_input_channels));
}

int wanda_component::get_max_input_channels(int con_point) const
{
    if (con_point <= num_input_channels && con_point > 0)
    {
        return max_input_channels[con_point - 1];
    }
    throw std::invalid_argument(get_complete_name_spec() +
                                " - Invalid connection point number: " + std::to_string(con_point) +
                                ". Number of inputs: " + std::to_string(num_input_channels));
}

void wanda_component::change_profile_tab(std::string choice, std::vector<float> globvars)
{
    if (!is_pipe())
    {
        throw std::runtime_error(get_complete_name_spec() + " is not a pipe");
    }
    auto &geom_in = get_property("Geometry input");
    auto cur_setting = geom_in.get_scalar_str();
    if (cur_setting == choice)
    {
        return;
    }
    //recalculate_hcs(globvars);
    auto &table = get_property("Profile").get_table();
    if (cur_setting == "Length" || cur_setting == "l-h")
    {
        if (choice == "xyz")
        {
            auto l = table.get_float_column("X-distance");
            auto h = table.get_float_column("Height");
            std::vector<float> y(l.size());
            std::fill(std::begin(y), std::end(y), 0.0);
            table.set_float_column("X-abs", l);
            table.set_float_column("Y-abs", y);
            table.set_float_column("Z-abs", h);
            // filling also the other collumns to ensure they size is ok, they will be overwritten by hcs calc.
            table.set_float_column("X-diff", l);
            table.set_float_column("Y-diff", l);
            table.set_float_column("Z-diff", l);
        }
        if (choice == "xyz dif")
        {
            auto l = table.get_float_column("X-distance");
            auto h = table.get_float_column("Height");
            if (l.empty())
            {
                return;
            }
            std::vector<float> x_diff, y_diff, z_diff;
            x_diff.push_back(l[0]);
            y_diff.push_back(0.0);
            z_diff.push_back(h[0]);
            for (int i = 1; i < l.size(); i++)
            {
                x_diff.push_back(l[i] - l[i - 1]);
                y_diff.push_back(0.0);
                z_diff.push_back(h[i] - h[i - 1]);
            }
            table.set_float_column("X-diff", x_diff);
            table.set_float_column("Y-diff", y_diff);
            table.set_float_column("Z-diff", z_diff);
            // filling also the other columns to ensure they size is ok, they will be overwritten by hcs calc.
            table.set_float_column("X-abs", x_diff);
            table.set_float_column("Y-abs", x_diff);
            table.set_float_column("Z-abs", x_diff);
        }
    }
    if (cur_setting == "xyz")
    {
        // other options are already filled by fill_profile_tab function
        if (choice == "xyz dif")
        {
            auto x = table.get_float_column("X-abs");
            auto y = table.get_float_column("Y-abs");
            auto z = table.get_float_column("Z-abs");
            if (x.empty())
            {
                return;
            }
            std::vector<float> x_diff, y_diff, z_diff;
            x_diff.push_back(x[0]);
            y_diff.push_back(y[0]);
            z_diff.push_back(z[0]);
            for (int i = 1; i < x.size(); i++)
            {
                x_diff.push_back(x[i] - x[i - 1]);
                y_diff.push_back(y[i] - y[i - 1]);
                z_diff.push_back(z[i] - z[i - 1]);
            }
            table.set_float_column("X-diff", x_diff);
            table.set_float_column("Y-diff", y_diff);
            table.set_float_column("Z-diff", z_diff);
        }
    }
    geom_in.set_scalar(choice);
   // recalculate_hcs(globvars); // TODO  waarom roepen we calc_HCS hier opnieuw aan?
}

float wanda_component::get_area()
{
    if (contains_property("Cross section"))
    {
        if (get_property("Cross section").get_scalar_str() == "Rectangle")
        {
            if (get_property("Inner width").get_spec_status())
            {
                if (get_property("Inner height").get_spec_status())
                {
                    if (get_property("Fillet size").get_spec_status())
                    {
                        float area = get_property("Inner width").get_scalar_float() *
                                     get_property("Inner height").get_scalar_float();
                        area -= 2.0f * pow(get_property("Fillet size").get_scalar_float(), 2.0f);
                        return area;
                    }
                    throw std::runtime_error("Fillet size of component " + get_complete_name_spec() +
                                             " not filled in yet");
                }
                throw std::runtime_error("Inner height of component " + get_complete_name_spec() +
                                         " not filled in yet");
            }
            throw std::runtime_error("Inner width of component " + get_complete_name_spec() + " not filled in yet");
        }
    }
    if (contains_property("Inner diameter"))
    {
        if (get_property("Fillet size").get_spec_status())
        {
            return std::numbers::pi_v<float> * pow(get_property("Inner diameter").get_scalar_float(), 2.0f) / 4.0f;
        }
        throw std::runtime_error("Inner diameter of component " + get_complete_name_spec() + " not filled in yet");
    }
    throw std::runtime_error(get_complete_name_spec() + " has no area");
}

void wanda_component::initialize()
{
    if (_component_definition->is_physical_component(_class_sort_key))
    {
        properties = _component_definition->get_properties(_class_sort_key);

        // auto input_properties =
        // componentDefinition->get_physical_input_properties(_class_sort_key); for
        // (auto& prop : input_properties)
        //    properties.emplace(prop.second.get_description(), prop.second);
        std::vector<int> inputs = _component_definition->get_num_input_props(_class_sort_key);
        _num_common_specs = inputs[0];
        _num_oper_specs = inputs[1];
        _num_hcs = inputs[2];
        _number_of_connnect_points = _component_definition->get_number_of_con_points(_class_sort_key);
        _is_controlable = _component_definition->get_controlable(_class_sort_key);
        // auto calculated_properties =
        // componentDefinition->get_physical_calc_properties(_class_sort_key); auto
        // output_properties =
        // componentDefinition->get_physical_output_properties(_class_sort_key);

        set_ctrl_input_type(_component_definition->get_ctrl_input_type(_class_sort_key));
        // for (auto& prop : calculated_properties)
        //    properties.emplace(prop.second.get_description(), prop.second);
        for (auto &prop : properties)
        {
            if (prop.second.is_glo_quant())
            {
                std::string temp =
                    (prop.second.get_description()).substr((prop.second.get_description()).size() - 1, 1);
                int con = strtol(temp.c_str(), nullptr, 10);
                if (con == 0)
                {
                    prop.second.set_number_of_elements(_num_elements);
                }
            }
            // properties.emplace(prop.second.get_description(), prop.second);
        }
        _core_quantities = _component_definition->get_core_quants(_class_sort_key);
    }
    else if (_component_definition->is_control_component(_class_sort_key))
    {
        auto input_properties = _component_definition->get_control_input_properties(_class_sort_key);
        auto output_properties = _component_definition->get_control_output_properties(_class_sort_key);
        for (auto &prop : input_properties)
            properties.emplace(prop.second.get_description(), prop.second);
        for (auto &prop : output_properties)
            properties.emplace(prop.second.get_description(), prop.second);
        std::vector<int> number_of_in_channels = _component_definition->get_num_input_chanl(_class_sort_key);
        std::vector<int> number_of_out_channels = _component_definition->get_num_output_chanl(_class_sort_key);
        num_input_channels = number_of_in_channels[0];
        num_output_channels = number_of_out_channels[0];
        min_input_channels = _component_definition->get_min_in_chan(_class_sort_key);
        max_input_channels = _component_definition->get_max_in_chan(_class_sort_key);

        if (_class_sort_key == "SENSOR")
        {
            _number_of_connnect_points = 1;
        }
    }
    else
    {
        throw std::invalid_argument("Unknow class sort key in case file. Class sort: " + _class_sort_key +
                                    " key: " + std::to_string(_component_key));
    }
}

void wanda_component::set_flipped(bool isFlipped)
{
    _is_flipped = isFlipped;
}

float wanda_component::get_angle() const
{
    return _shape_angle * 180 / 3.14159265358979f;
}

float wanda_component::get_angle_rad() const
{
    return _shape_angle;
}

bool wanda_component::get_is_flipped() const
{
    return _is_flipped;
}

std::string wanda_component::get_core_quants(const int con_point) const
{
    return _core_quantities[con_point - 1];
}

bool wanda_component::is_sigline_connected(const int connection_point, const bool is_input)
{
    if (is_input)
        return (_connected_siglines_input.find(connection_point) != _connected_siglines_input.end());
    return (_connected_siglines_output.find(connection_point) != _connected_siglines_output.end());
}

wanda_node &wanda_component::get_connected_node(const int connection_point)
{
    if (connection_point > _number_of_connnect_points)
    {
        throw std::invalid_argument("Connect point number greater then number of connection points of "
                                    "component " +
                                    get_complete_name_spec());
    }
    if (_connected_nodes.find(connection_point) != _connected_nodes.end())
        return *(_connected_nodes[connection_point]);
    throw std::invalid_argument("No node connected to that connection point");
}

std::vector<wanda_node *> wanda_component::get_connected_nodes()
{
    std::vector<wanda_node *> vec;
    for (auto &it : _connected_nodes)
    {
        vec.push_back(it.second);
    }
    return vec;
}

std::vector<wanda_sig_line *> wanda_component::get_connected_sigline(const int connection_point, const bool is_input)
{
    std::vector<wanda_sig_line *> lines;
    std::multimap<int, wanda_sig_line *> *data;
    if (is_input)
        data = &_connected_siglines_input;
    else
        data = &_connected_siglines_output;
    if (data->empty())
    {
        return lines;
    }
    auto range = data->equal_range(connection_point);
    if (range.first == data->end())
        throw std::invalid_argument("No signal lines connected to this connection point");
    for (auto i = range.first; i != data->end(); ++i)
    {
        lines.push_back(i->second);
    }
    return lines;
}

int wanda_component::get_connect_point(wanda_item &node)
{
    for (auto con_point : _connected_nodes)
    {
        if (con_point.second->get_complete_name_spec() == node.get_complete_name_spec())
        {
            return con_point.first;
        }
    }
    for (auto con_point : _connected_siglines_input)
    {
        if (con_point.second->get_complete_name_spec() == node.get_complete_name_spec())
        {
            return con_point.first;
        }
    }
    for (auto con_point : _connected_siglines_output)
    {
        if (con_point.second->get_complete_name_spec() == node.get_complete_name_spec())
        {
            return con_point.first;
        }
    }
    throw std::runtime_error(node.get_complete_name_spec() + " not connected to component " + get_complete_name_spec());
}

void wanda_component::set_input_channel_type(std::vector<std::string> in_chan_t)
{
    input_chan_type = in_chan_t;
}

//! returns the input channel type of the given connection point
/*!
 \param connection_point is connection point for which the type is returned
 */
std::string wanda_component::get_input_channel_type(int connection_point) const
{
    if (connection_point > num_input_channels)
    {
        throw std::invalid_argument(get_complete_name_spec() + " has only " + std::to_string(num_input_channels) +
                                    " and not " + std::to_string(connection_point));
    }
    return input_chan_type[connection_point - 1];
}

void wanda_component::set_output_channel_type(std::vector<std::string> in_chan_t)
{
    output_chan_type = in_chan_t;
}

//! returns the output channel type of the given connection point
/*!
\param connection_point is connection point for which the type is returned
*/
std::string wanda_component::get_output_channel_type(int connection_point) const
{
    if (connection_point > num_output_channels)
    {
        throw std::invalid_argument(get_complete_name_spec() + " has only " + std::to_string(num_output_channels) +
                                    " and not " + std::to_string(connection_point));
    }
    return output_chan_type[connection_point - 1];
}

// only used for SENSOR components
// is automaticly called when a sensor is connected
void wanda_component::fill_sensor_list(wanda_component comp, int con_point)
{
    std::vector<std::string> list;
    if (comp.get_number_of_connnect_points() < con_point)
    {
        if (comp.is_pipe())
        {
            list = _component_definition->get_list_quant_names(comp.get_class_sort_key());
        }
        size_t noo = comp.get_number_of_outputs();
        size_t size = list.size();
        list.resize(size + noo);
        for (auto input : comp.properties)
        {
            if (input.second.get_property_type() == wanda_property_types::HOS)
            {
                list[size + input.second.get_hos_index()] = input.first;
            }
        }
    }
    else
    {
        list = _component_definition->get_list_quant_names(comp.get_class_sort_key());
    }
    properties["Quantity"].set_list(list);
}

void wanda_component::fill_sensor_list(wanda_node const node)
{
    std::vector<std::string> list = _component_definition->get_list_quant_names(node.get_class_sort_key());
    for (auto prop : node)
    {
        if (prop.second.get_property_type() == wanda_property_types::NOS)
        {
            list.push_back(prop.second.get_description());
        }
    }
    properties["Quantity"].set_list(list);
}

// should not be called by user, use Wandamodel function
void wanda_component::connect(wanda_node &item, int connection_point)
{
    if (this->get_item_type() == wanda_type::physical && item.get_item_type() != wanda_type::node)
    {
        throw std::invalid_argument("Invalid connection: " + this->get_complete_name_spec() + " and " +
                                    item.get_complete_name_spec());
    }
    if (get_item_type() == wanda_type::control && this->get_class_sort_key() != "SENSOR")
    // TODO check if this is possible if you pass a node
    {
        throw std::invalid_argument("Invalid connection: " + this->get_complete_name_spec() + " and " +
                                    item.get_complete_name_spec());
    }
    _connected_nodes.emplace(connection_point, &item);
    this->set_modified(true);
}

void wanda_component::connect(wanda_sig_line &sig_line, int connection_point, bool is_input)
{
    if (is_input)
    {
        _connected_siglines_input.emplace(connection_point, &sig_line);
    }
    else
    {
        _connected_siglines_output.emplace(connection_point, &sig_line);
    }

    this->set_modified(true);
}

void wanda_component::disconnect(int connection_point)
{
    _connected_nodes.erase(connection_point);
    this->set_modified(true);
}

void wanda_component::disconnect(wanda_node &node)
{
    int connectionpoint = 0;
    for (auto &item : _connected_nodes)
    {
        if (item.second->get_key() == node.get_key())
        {
            connectionpoint = item.first;
            break;
        }
    }
    if (connectionpoint == 0)
        throw std::invalid_argument("Node is not connected to this component");
    _connected_nodes.erase(connectionpoint);
    this->set_modified(true);
}

void wanda_component::disconnect(int connection_point, bool input)
{
    if (input)
    {
        _connected_siglines_input.erase(connection_point);
    }
    else if (!input)
    {
        _connected_siglines_output.erase(connection_point);
    }
    this->set_modified(true);
}

void wanda_component::disconnect(wanda_sig_line &sig_line)
{
    for (auto item : _connected_siglines_input)
    {
        if (item.second->get_key() == sig_line.get_key())
        {
            _connected_siglines_input.erase(item.first);
            this->set_modified(true);
            return;
        }
    }
    for (auto item : _connected_siglines_output)
    {
        if (item.second->get_key() == sig_line.get_key())
        {
            _connected_siglines_output.erase(item.first);
            this->set_modified(true);
            return;
        }
    }
    throw std::invalid_argument("signal line " + sig_line.get_complete_name_spec() +
                                " is not connected to this component");
}

void wanda_component::fill_profile_table()
{
    // fills the profile table of a pipe.
    auto &xyztab = get_property("Profile").get_table();
    if (is_node_connected(1) && is_node_connected(2) &&
        get_property("Geometry input").get_selected_item() == "Length" && get_property("Length").get_spec_status() &&
        get_property("Length").is_modified() && get_connected_node(1).get_property("Elevation").get_spec_status() &&
        get_connected_node(2).get_property("Elevation").get_spec_status())
    {
        std::vector<float> x(2);
        std::vector<float> y(2);
        std::vector<float> s(2);
        x[0] = 0.0;
        s[0] = 0.0;
        y[0] = get_connected_node(1).get_property("Elevation").get_scalar_float();

        s[1] = get_property("Length").get_scalar_float();
        y[1] = get_connected_node(2).get_property("Elevation").get_scalar_float();
        x[1] = sqrt(pow(s[1], 2.0f) - pow(y[1] - y[0], 2.0f));
        get_property("Profile").get_table().set_float_column("X-distance", x);
        get_property("Profile").get_table().set_float_column("Height", y);
        get_property("Profile").get_table().set_float_column("S-distance", s);
        get_property("Profile").get_table().set_modified(true);
    }
    else if (get_property("Geometry input").get_selected_item() == "l-h" &&
             get_property("Profile").get_table().is_modified())
    {
        std::vector<float> l = get_property("Profile").get_table().get_float_column("X-distance");
        std::vector<float> h = get_property("Profile").get_table().get_float_column("Height");
        std::vector<float> s;
        s.push_back(0.0f);
        for (size_t i = 1; i < l.size(); i++)
        {
            s.push_back(s.back() + sqrt(pow(l[i] - l[i - 1], 2.0f) + pow(h[i] - h[i - 1], 2.0f)));
        }
        xyztab.set_float_column("S-distance", s);
        get_property("Length").set_scalar(s.back());
    }
    else if (get_property("Geometry input").get_selected_item() == "xyz" &&
             get_property("Profile").get_table().is_modified())
    {
        if (xyztab.get_float_column("X-abs").size() > 1)
        {
            std::vector<float> l;
            std::vector<float> h;
            std::vector<float> s;
            std::vector<float> x_diff;
            std::vector<float> y_diff;
            std::vector<float> z_diff;
            auto x = xyztab.get_float_column("X-abs");
            auto y = xyztab.get_float_column("Y-abs");
            h = xyztab.get_float_column("Z-abs");
            x_diff.push_back(x[0]);
            y_diff.push_back(y[0]);
            z_diff.push_back(h[0]);
            s.push_back(0.0);
            l.push_back(0.0);
            for (size_t i = 1; i < x.size(); i++)
            {
                s.push_back(s[i - 1] +
                            sqrt(pow(x[i] - x[i - 1], 2.0f) + pow(y[i] - y[i - 1], 2.0f) + pow(h[i] - h[i - 1], 2.0f)));
                l.push_back(l[i - 1] + sqrt(pow(x[i] - x[i - 1], 2.0f) + pow(y[i] - y[i - 1], 2.0f)));
                x_diff.push_back(x[i] - x[i - 1]);
                y_diff.push_back(y[i] - y[i - 1]);
                z_diff.push_back(h[i] - h[i - 1]);
            }
            xyztab.set_float_column("X-distance", l);
            xyztab.set_float_column("Height", h);
            xyztab.set_float_column("S-distance", s);
            xyztab.set_float_column("X-diff", x_diff);
            xyztab.set_float_column("Y-diff", y_diff);
            xyztab.set_float_column("Z-diff", z_diff);
            xyztab.set_modified(true);
            get_property("Length").set_scalar(s.back());
        }
    }
    else if (get_property("Geometry input").get_selected_item() == "xyz dif" &&
             get_property("Profile").get_table().is_modified())
    {
        std::vector<float> l;
        std::vector<float> h;
        std::vector<float> s;
        std::vector<float> x_abs;
        std::vector<float> y_abs;
        std::vector<float> z_abs;
        auto x_diff = xyztab.get_float_column("X-diff");
        auto y_diff = xyztab.get_float_column("Y-diff");
        auto z_diff = xyztab.get_float_column("Z-diff");
        l.push_back(0.0);
        h.push_back(z_diff[0]);
        s.push_back(0.0);
        x_abs.push_back(x_diff[0]);
        y_abs.push_back(y_diff[0]);
        z_abs.push_back(z_diff[0]);
        for (int i = 1; i < x_diff.size(); i++)
        {
            l.push_back(l.back() + sqrt(pow(x_diff[i], 2.0f) + pow(y_diff[i], 2.0f)));
            h.push_back(h.back() + z_diff[i]);
            s.push_back(s.back() + sqrt(pow(x_diff[i], 2.0f) + pow(y_diff[i], 2.0f) + pow(z_diff[i], 2.0f)));
            x_abs.push_back(x_abs.back() + x_diff[i]);
            y_abs.push_back(y_abs.back() + y_diff[i]);
            z_abs.push_back(z_abs.back() + z_diff[i]);
        }
        xyztab.set_float_column("X-distance", l);
        xyztab.set_float_column("Height", h);
        xyztab.set_float_column("S-distance", s);
        xyztab.set_float_column("X-abs", x_abs);
        xyztab.set_float_column("Y-abs", y_abs);
        xyztab.set_float_column("Z-abs", z_abs);
        get_property("Length").set_scalar(s.back());
    }
}


void wanda_component::set_material_name(std::string material)
{
    if (!is_pipe())
    {
        throw std::runtime_error(get_complete_name_spec() + " is not a pipe");
    }

    if (material.size() > 24)
    {
        throw std::length_error("String is longer than 24 characters");
    }
    _material_name = material;
    _is_modified = true;
}

std::vector<float> wanda_component::get_his_values() const
{
    std::vector<float> input_values(_num_common_specs + _num_oper_specs);
    for (auto &wanda_prop : properties)
    {
        if(!wanda_prop.second.is_input()) continue;
        if (wanda_prop.first == "Action table") continue;
        int const index =
            (wanda_prop.second.get_property_spec_code() == 'C' ? 0 : _num_common_specs) + wanda_prop.second.get_index();        
        input_values[index] = wanda_prop.second.get_spec_status() ? wanda_prop.second.get_scalar_float() : -999;        
    }
    return input_values;
}

std::vector<float> wanda_component::get_height_nodes()
{
    std::vector<float> height;
    for (int i = 0; i < _number_of_connnect_points;i++)
    {
        if (!is_node_connected(i+1))
        { //no node connected to this connect point
            height.push_back(-999);
            continue;
        }
        if (!get_connected_node(i + 1).get_property("Elevation").get_spec_status())
        {
            //height has not be filled in yet
            height.push_back(-999);
            continue;
        }
        height.push_back(get_connected_node(i+1).get_property("Elevation").get_scalar_float());       
    }
    return height;
}

table_data wanda_component::get_table_data()
{
    table_data result(_num_common_specs + _num_oper_specs);
    for (auto &wanda_prop : properties)
    {
        if (!wanda_prop.second.has_table()) continue;
        if (wanda_prop.first == "Action table") continue;
       
        auto &table = wanda_prop.second.get_table();
        for (auto& description:table.get_descriptions())
        {
            auto tab_data = table.get_table_data(description);
            int const index = (wanda_prop.second.get_property_spec_code() == 'C' ? 0 : _num_common_specs) + tab_data->_index;
            if (tab_data->floattable.empty() || tab_data->_table_type == 'S')
                continue;
            
            // TT2 contains pointers to the second column of a table. TT1
            // contains pointers to the rest.
            if (tab_data->_table_type == 'T' && tab_data->_col_num == 1)
            {
                result.column2[index] = tab_data->floattable.data();
            }
            else
            {
                result.column1[index ] = tab_data->floattable.data();
            }
            result.size_tables[index] = tab_data->floattable.size();
        }
    }
    return result;
}

int wanda_component::get_number_hsc() const
{
    int num_hcs = 0;
    for (auto& prop: properties)
    {
        if (prop.second.get_property_type() == wanda_property_types::HCS)
            num_hcs++;
    }
    return num_hcs;
}

void wanda_component::set_hsc_results(std::vector<float> results)
{   
    for (auto &prop : properties)
    {
        if (prop.second.get_property_type() != wanda_property_types::HCS)
        {
            continue;            
        }
        if (results[prop.second.get_index()] < -1e20)
        {
            //If compoennt dll could not calcualte a value it is set to -1e30.
            continue;
        }
        prop.second.set_scalar(results[prop.second.get_index()]);
    }
    if (contains_property("Pipe element count")) 
    {
        set_num_elements(get_property("Pipe element count").get_scalar_float());
    }
}
    

void wanda_component::set_length_from_hsc(std::vector<float> his)
{
    if (!contains_property("Length"))
        return;
    auto &prop = get_property("Length");
    int index = prop.get_index() +  (prop.get_property_spec_code() == 'C' ? 0: _num_common_specs);
    prop.set_scalar(his[index]);
}
