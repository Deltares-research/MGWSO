#include <stdexcept>
#include <string>
#include <wandacomponent.h>
#include <wandasigline.h>

const std::string wanda_sig_line::_object_name = "WandaSignalLine Object";

wanda_sig_line::wanda_sig_line(int signal_key, std::string name)
    : wanda_item(signal_key, "Signal", "Signal", "Signal", name, wanda_type::signal_line, "Signal line"),
      _object_hash(std::hash<std::string>{}(_object_name))
{
    _group_index = 0;
    _connected_comp_output = nullptr;
    _connected_comp_input = nullptr;
    _input_chan_num = 0;
    _output_chan_num = 0;
}

wanda_sig_line::wanda_sig_line()
    : wanda_item(-999, "Signal", "Signal", "Signal", "empty", wanda_type::signal_line, "Signal line"),
      _object_hash(std::hash<std::string>{}(_object_name))
{
    _group_index = 0;
    _connected_comp_output = nullptr;
    _connected_comp_input = nullptr;
    _input_chan_num = 0;
    _output_chan_num = 0;
}

std::vector<int> wanda_sig_line::get_con_points() const
{
    return con_point;
}

void wanda_sig_line::set_con_point(std::vector<int> con_points)
{
    con_point = con_points;
}

void wanda_sig_line::connect_input(wanda_component &component, int connection_point)
{
    if (component.get_item_type() == wanda_type::control)
    {
        if (_signal_line_type.compare(component.get_input_channel_type(connection_point)) != 0)
            throw std::invalid_argument("Cannot connect signal line of type " + _signal_line_type + " to " +
                                        component.get_input_channel_type(connection_point) + " connection point");
        _connected_comp_input = &component;
        _input_chan_num = connection_point;
    }
    if (component.get_item_type() == wanda_type::physical)
    {
        if (_signal_line_type.compare(component.get_ctrl_input_type()) != 0 &&
            component.get_ctrl_input_type().compare("both") != 0)
        {
            throw std::invalid_argument("Cannot connect signal line of type " + _signal_line_type + " to " +
                                        component.get_ctrl_input_type() + " connection point");
        }
        if (connection_point != component.get_number_of_connnect_points() + 1)
        {
            throw std::invalid_argument("Can only connect signal line to control connect point of hydraulic "
                                        "component");
        }
        _connected_comp_input = &component;
        _input_chan_num = connection_point;
    }
}

void wanda_sig_line::connect_output(wanda_component &component, int connection_point)
{
    if (component.get_item_type() == wanda_type::control)
    {
        if (_signal_line_type.compare(component.get_output_channel_type(connection_point)) != 0)
            throw std::invalid_argument("Cannot connect signal line of type " + _signal_line_type + " to " +
                                        component.get_output_channel_type(connection_point) + " connection point");
    }
    if (component.get_item_type() == wanda_type::physical)
    {
        if (_signal_line_type.compare("sensor") != 0)
        {
            throw std::invalid_argument("Can only attach a sensor to a hydraulic component connect point: " +
                                        component.get_complete_name_spec() +
                                        " connection point: " + std::to_string(connection_point));
        }
    }
    _output_chan_num = connection_point;
    _connected_comp_output = &component;
}
