#include <functional>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <wanda_item.h>
#include <wandadef.h>

const std::string wanda_item::_object_name = "WandaItem Object";

bool wanda_item::has_keyword(std::string keyword) const
{
    auto const result = std::find(_keywords.begin(), _keywords.end(), keyword);
    if (result != _keywords.end())
    {
        return true;
    }
    return false;
}

void wanda_item::remove_keyword(std::string keyword)
{
    if (!has_keyword(keyword))
    {
        throw ::std::invalid_argument(get_complete_name_spec() + " has no keyword " + keyword);
    }
    auto const result = std::find(_keywords.begin(), _keywords.end(), keyword);
    _keywords.erase(result);
    set_modified(true);
}

bool wanda_item::is_action_table_used()
{
    if (has_action_table())
    {
        return _action_table_used;
    }
    throw std::runtime_error("Component has no action table");
}

bool wanda_item::has_action_table() const
{
    return properties.find("Action table") != properties.end();
}

void wanda_item::set_disused(bool status)
{
    _disused = status;
    _is_modified = true;
    for (auto &prop : properties)
    {
        if (prop.second.has_series())
        {
            prop.second.set_disused(status);
        }
    }
}

void wanda_item::add_keyword(std::string keywordin)
{

    _keywords.push_back(keywordin);
    _is_modified = true;
    // determine the size of the keywords group
    std::size_t size = 0;
    for (auto const keyword : _keywords)
    {
        size += keyword.size() + 1;
    }
    // if (size > 50)
    //    throw std::runtime_error("size of keywords exceeds limit of 50 characters");
}

bool wanda_item::contains_property(const std::string &property_description)
{
    if (properties.find(property_description) != properties.end())
        return true;
    return false;
}

wanda_property &wanda_item::get_property(std::string property_description)
{
    if (properties.find(property_description) != properties.end())
    {
        if (num_of_species != nullptr)
        {
            if (properties[property_description].get_species_number() > *num_of_species)
            {
                throw std::invalid_argument("Wanda case only has " + std::to_string(*num_of_species) + " " +
                                            property_description + " is not existing in this case");
            }
        }
        return properties[property_description];
    }
    throw std::invalid_argument("Property doesn't exist: " + property_description);
}

void wanda_item::add_message(std::string message_in, char type, float time)
{
    wanda_message message;
    message.message = message_in;
    message.time = time;
    message.message_type = type;
    _messages.push_back(message);
}

std::vector<std::string> wanda_item::get_messages()
{
    std::vector<std::string> message;
    for (auto mes : _messages)
    {
        message.push_back(mes.message);
    }
    return message;
}

std::vector<float> wanda_item::get_message_time()
{
    std::vector<float> message_time;
    for (auto mes : _messages)
    {
        message_time.push_back(mes.time);
    }
    return message_time;
}

std::vector<char> wanda_item::get_message_type()
{
    std::vector<char> message_type;
    for (auto mes : _messages)
    {
        message_type.push_back(mes.message_type);
    }
    return message_type;
}

std::vector<std::string> wanda_item::validate_input(int view_mask_model)
{
    // TODO check of tables?
    std::vector<std::string> result;
    for (auto &property : properties)
    {
        if (property.second.is_input() && property.first != "Action table" &&
            property.second.get_input_type_code() != 'J' && (property.second.get_view_mask() & view_mask_model) != 0)
        {
            if (!property.second.get_spec_status())
            {
                std::string rel_his = property.second.get_list_dependency();
                auto list_num = property.second.get_view_list_numbers();
                if (rel_his == "NoDependency")
                {
                    if (property.second.has_table())
                    {
                        if (property.second.get_table().check_table())
                        {
                            continue;
                        }
                    }
                    result.push_back(property.first);
                    continue;
                }
                while (rel_his != "NoDependency")
                {
                    auto prop = get_property(rel_his);
                    if (std::find(list_num.begin(), list_num.end(), prop.get_scalar_float()) == list_num.end())
                    {
                        break;
                    }
                    rel_his = prop.get_list_dependency();
                    list_num = prop.get_view_list_numbers();
                    if (rel_his == "NoDependency")
                    {
                        if (property.second.has_table())
                        {
                            if (!property.second.get_table().check_table())
                            {
                                result.push_back(property.first);
                            }
                        }
                        else
                        {
                            result.push_back(property.first);
                        }
                        break;
                    }
                }
            }
        }
        else if (property.first == "Action table")
        {
            if (is_action_table_used())
            {
                if (!property.second.get_table().check_table())
                {
                    result.push_back(property.first);
                }
            }
            // if it is a TAB it has a property named action table and this needs to
            // be checked
            if (get_type_name() == "Time table")
            {
                if (!property.second.get_table().check_table())
                {
                    result.push_back(property.first);
                }
            }
        }
    }
    return result;
}

std::vector<wanda_property *> wanda_item::get_all_properties()
{
    std::vector<wanda_property *> list;
    for (auto &item : properties)
    {
        list.push_back(&item.second);
    }

    return list;
}

void wanda_item::add_data_from_template(std::unordered_map<std::string, wanda_prop_template> data)
{
    for (auto item : data)
    {
        if (contains_property(item.first))
        {
            auto &prop = get_property(item.first);
            prop.set_value_from_template(item);
        }
    }
}

void wanda_item::copy_input(wanda_item &item)
{
    if (item.has_action_table())
    {
        _action_table_used = item.is_action_table_used();
    }
    for (auto &property : item)
    {
        // has the property data
        if (!property.second.get_spec_status())
        {
            continue;
        }
        // does it exist in current comp
        if (!contains_property(property.first))
        {
            continue;
        }
        // copy
        auto &prop_new = get_property(property.first);
        prop_new.copy_data(property.second);
    }
}

int wanda_item::get_number_of_outputs()
{
    int noo = 0;
    for (auto &items : properties)
    {
        if (items.second.is_output())
        {
            noo++;
        }
    }
    return noo;
}

wanda_item::wanda_item(int compkey, std::string classname, std::string cskey, std::string nameprefix, std::string name,
                       wanda_type type, std::string type_name)
    : _item_type(type), _disused(false), _is_modified(false), _action_table_used(false), _name(name),
      _component_key(compkey), _class_sort_key(cskey), _class_name(classname), _name_prefix(nameprefix),
      _item_position({0, 0}), _new_item(false), _group_index(0), _type_name(type_name),
      _object_hash_item(std::hash<std::string>{}(_object_name))
{
}

wanda_item::wanda_item(std::string class_sort_key, std::string name_pre_fix, wanda_type type, std::string def_mask,
                       std::string conv2comp, std::string type_name)
    : _item_type(type), _disused(false), _is_modified(false), _action_table_used(false), _name(""),
      _component_key(-999), _class_sort_key(class_sort_key), _class_name(""), _name_prefix(name_pre_fix),
      _item_position({0, 0}), _new_item(false), _group_index(0), _default_mask(def_mask), _convert2comp(conv2comp),
      _type_name(type_name), _object_hash_item(std::hash<std::string>{}(_object_name))
{
}

void wanda_item::set_name(std::string name)
{
    if (name.size() > 128)
    {
        throw std::length_error("String is longer than 128 characters: " + name);
    }
    _name = name;
    _is_modified = true;
}

std::string wanda_item::get_key_as_string() const
{
    std::stringstream ss;
    char type_spec = 'X';
    switch (_item_type)
    {
    case wanda_type::physical:
        type_spec = 'H';
        break;
    case wanda_type::control:
        type_spec = 'C';
        break;
    case wanda_type::node:
        type_spec = 'N';
        break;
    case wanda_type::signal_line:
        type_spec = 'S';
        break;
    }
    if (_component_key != -999)
    {
        ss << type_spec << std::setw(7) << std::setfill('0') << _component_key;
        return ss.str();
    }
    return "Unrefrnc";
}

void wanda_item::reset_modified()
{
    for (auto &prop : properties)
    {
        prop.second.set_modified(false);
        if (prop.second.has_table())
        {
            prop.second.get_table().set_modified(false);
        }
    }
    _is_modified = false;
}

void wanda_item::set_use_action_table(bool status)
{
    if (has_action_table())
    {
        _action_table_used = status;
        _is_modified = true;
        return;
    }
    throw std::runtime_error("Component has no action table");
}

void wanda_item::set_comment(std::string comment)
{
    if (comment.size() > 50)
    {
        throw std::length_error("Comment is longer than 50 characters: " + comment);
    }
    _comment = comment;
    _is_modified = true;
}

void wanda_item::set_user_name(std::string user_name)
{
    if (user_name.size() > 24)
    {
        throw std::length_error("String is longer than 24 characters: " + user_name);
    }
    _user_name = user_name;
    _is_modified = true;
}

void wanda_item::set_date_mod(std::string date)
{
    if (_date_modified.size() > 17)
    {
        throw std::length_error("String is longer than 17 characters");
    }
    _date_modified = date;
    _is_modified = true;
}

void wanda_item::set_model_name(std::string model_name)
{
    if (_item_type != wanda_type::physical)
    {
        throw std::runtime_error("Only physical components have a model name field");
    }
    if (model_name.size() > 24)
    {
        throw std::length_error("String is longer than 24 characters");
    }
    _model_name = model_name;
    _is_modified = true;
}

void wanda_item::set_ref_id(std::string ref_id)
{
    if (_ref_id.size() > 120)
    {
        throw std::length_error("String is longer than 120 characters");
    }
    _ref_id = ref_id;
    _is_modified = true;
}

void wanda_item::set_material_name(std::string material)
{
    throw std::runtime_error(get_complete_name_spec() + " is not a pipe");
}

bool wanda_item::is_modified() const
{
    if (_is_modified)
        return _is_modified;
    for (auto item : properties)
    {
        if (item.second.is_modified() && item.second.get_property_type() == wanda_property_types::HIS)
        {
            return true;
        }
        if (item.second.has_table())
        {
            if (item.second.get_table().is_modified())
            {
                return true;
            }
        }
    }
    return false;
}
