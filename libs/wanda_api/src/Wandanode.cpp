#include <stdexcept>
#include <wandacomponent.h>
#include <wandadef.h>
#include <wandanode.h>

const std::string wanda_node::_object_name = "WandaNode Object";

wanda_node::wanda_node(int compkey, std::string classname, std::string csKey, std::string namePrefix, std::string name,
                       std::string type_name, wanda_def *component_definition)
    : wanda_item(compkey, classname, csKey, namePrefix, name, wanda_type::node, type_name),
      _object_hash(std::hash<std::string>{}(_object_name))
{
    wanda_def *componentDefinition = component_definition;

    properties = componentDefinition->get_properties(_class_sort_key);
    _core_quantities.push_back(componentDefinition->get_node_core_quants(_class_sort_key));
    _node_type = componentDefinition->get_node_type(_class_sort_key);
}

wanda_node::wanda_node(std::string class_sort_key, wanda_type type, std::string name_pre_fix, std::string type_name,
                       std::string def_mask, std::string convert2comp, wanda_def *component_definition)
    : wanda_item(class_sort_key, name_pre_fix, type, def_mask, convert2comp, type_name),
      _object_hash(std::hash<std::string>{}(_object_name))
{
    wanda_def *componentDefinition = component_definition;
    properties = componentDefinition->get_properties(_class_sort_key);
    auto node_core_quants = componentDefinition->get_node_core_quants(_class_sort_key);
    _core_quantities.push_back(node_core_quants);
    _node_type = componentDefinition->get_node_type(_class_sort_key);
}

std::vector<wanda_component *> wanda_node::get_connected_components() const
{
    std::vector<wanda_component *> list;
    for (auto &comp : _connected_comps)
        list.push_back(comp);
    return list;
}

wanda_node::wanda_node()
    : wanda_item(0, "empty", "empty", "empty", "empty", wanda_type::node, "empty"),
      _object_hash(std::hash<std::string>{}(_object_name))
{
}

void wanda_node::connect(wanda_component &component)
{
    if (component.get_item_type() != wanda_type::physical)
        if (component.get_class_sort_key().compare("SENSOR") != 0)
            throw std::invalid_argument("Can only connect physical components");
    if (std::find(_connected_comps.begin(), _connected_comps.end(), &component) == _connected_comps.end())
    {
        _connected_comps.push_back(&component);
        this->set_modified(true);
    }
}

void wanda_node::disconnect(wanda_component &component)
{

    int i = 0;
    for (auto &con_comp : _connected_comps)
    {
        if (con_comp->get_complete_name_spec() == component.get_complete_name_spec())
        {
            _connected_comps.erase(_connected_comps.begin() + i);
            this->set_modified(true);
            return;
        }
        i++;
    }
}

void wanda_node::disconnect()
{
    _connected_comps.clear();
    this->set_modified(true);
}
