#ifndef _WANDAITEM_
#define _WANDAITEM_

#include "Wandaproperty.h"
#include <unordered_map>
#include <vector>
#include <wanda_diagram_lines.h>

#ifdef WANDAMODEL_EXPORT
// #define WANDAMODEL_API __declspec(dllexport)
#define WANDAMODEL_API 
#else
#define WANDAMODEL_API __declspec(dllimport)
#endif

class wanda_component;
enum class wanda_type
{
    physical,
    control,
    node,
    signal_line
};

///@private
struct wanda_message
{
    std::string message;
    char message_type{};
    float time{};
};


//!  main class for a Wanda item.
/*!
The wanda_item class is the main interface for wanda components, nodes and
signal lines. The classes for these are derived of the Wanda_item class. In the
Wanda item class, iterators and [] operators have been implemented to iterate or
access the properties of the component.
*/
class WANDAMODEL_API wanda_item
{
  protected:
    ///@private
    const static std::string _object_name;
    ///@private
    const std::size_t _object_hash_item;
    ///@private
    wanda_type _item_type;
    ///@private
    bool _disused = false;
    ///@private
    bool _is_modified = false;
    ///@private
    bool _action_table_used = false;
    ///@private
    std::string _name;
    ///@private
    int _component_key = -999;
    ///@private
    std::string _class_sort_key;
    ///@private
    std::string _class_name;
    ///@private
    std::string _type_string;
    ///@private
    std::string _name_prefix;
    ///@private
    std::vector<float> _item_position;
    ///@private
    std::string _comment;
    ///@private
    std::vector<std::string> _keywords;
    ///@private
    std::string _user_name;
    ///@private
    std::string _date_modified;
    ///@private
    std::string _model_name;
    ///@private
    int _sequence_number = 0;
    ///@private
    std::string _ref_id;
    ///@private
    std::string _material_name;
    ///@private
    std::unordered_map<std::string, wanda_property> properties;
    ///@private
    bool _new_item;
    ///@private
    int _group_index;
    ///@private
    int _oper_index;
    ///@private
    int _com_index;
    ///@private
    std::vector<std::string> _core_quantities; // core quantities for each connection point
                                               // (components). for nodes this container only has one
                                               // item
                                               ///@private
    std::vector<wanda_message> _messages;
    ///@private
    std::string _default_mask;
    ///@private
    std::string _convert2comp;
    ///@private
    std::string _type_name;
    ///@private
    int *num_of_species = nullptr;
    std::vector<wanda_diagram_lines*> lines_info;
  public:
    ///@private
    std::unordered_map<std::string, wanda_property>::iterator begin() noexcept
    {
        return properties.begin();
    }
    ///@private
    std::unordered_map<std::string, wanda_property>::const_iterator begin() const noexcept
    {
        return properties.begin();
    }
    ///@private
    std::unordered_map<std::string, wanda_property>::iterator end() noexcept
    {
        return properties.end();
    }
    ///@private
    std::unordered_map<std::string, wanda_property>::const_iterator end() const noexcept
    {
        return properties.end();
    }
    ///@private
    std::unordered_map<std::string, wanda_property>::iterator find(const std::string &k)
    {
        return properties.find(k);
    }
    ///@private
    std::unordered_map<std::string, wanda_property>::const_iterator find(const std::string &k) const
    {
        return properties.find(k);
    }
    ///@private
    wanda_property &operator[](const std::string &k)
    {
        return properties[k];
    }
    ///@private
    wanda_property &operator[](std::string &&k)
    {
        return properties[k];
    }
    ///@private
    wanda_item(int compkey, std::string classname, std::string cskey, std::string nameprefix, std::string name,
               wanda_type type, std::string type_name);
    ///@private
    wanda_item(std::string class_sort_key, std::string name_pre_fix, wanda_type type, std::string def_mask,
               std::string conv2comp, std::string type_name);
    ///@private
    virtual ~wanda_item() = default;
    ///@private
    virtual wanda_type get_item_type() const
    {
        return _item_type;
    }
    ///@private
    virtual void set_name(std::string name);
    ///@private
    virtual void set_class_name(const std::string &class_name)
    {
        _class_name = class_name;
    }
    ///@private
    virtual void set_comp_key(int key)
    {
        _component_key = key;
    }
    ///@private
    virtual int get_key() const
    {
        return _component_key;
    }
    ///@private
    virtual int get_sequence_number() const
    {
        return _sequence_number;
    }
    ///@private
    virtual int get_group_index()
    {
        return _group_index;
    }
    ///@private
    virtual int get_com_index()
    {
        return _com_index;
    }
    ///@private
    virtual int get_oper_index()
    {
        return _oper_index;
    }
    //! Returns true when disused and false when not.
    virtual bool is_disused() const
    {
        return _disused;
    };
    //! Returns true when the component has the keyword
    /*!
     \param keyword to check.
     */
    virtual bool has_keyword(std::string keyword) const;
    //! Removes keyword from the keyword list
    /*!
    \param keyword to remove.
    */
    virtual void remove_keyword(std::string keyword);
    //! clears the keywords of the component
    virtual void clear_keywords()
    {
        _keywords.clear();
    }
    ///@private
    virtual bool is_modified() const;
    ///@private
    virtual bool is_new() const
    {
        return _new_item;
    }
    //! Returns true when the action table of the component is used.
    virtual bool is_action_table_used();
    //! Returns true when the component has an action table
    virtual bool has_action_table() const;
    //! Sets disuse to the value provided by the user
    /*!
     \param status to set the component to, disused (true) or use (false)
     */
    virtual void set_disused(bool status);
    //! Adds keyword to the component
    /*!
     Adds the given keyword to the component. The total lenght of the list of
    keyword should be less then 50 which is the maximum possible length for the
    keyword list \param keyword to add to the list.
     */
    virtual void add_keyword(std::string keyword);
    ///@private
    virtual void set_modified(bool status)
    {
        _is_modified = status;
    }
    ///@private
    virtual void reset_modified();
    ///@private
    virtual void set_new(bool status)
    {
        _new_item = status;
    }
    //! Sets the status of the action table of the component
    /*!
     \param status is true when the action table should be switched on and false
     when it should be switched off.
     */
    virtual void set_use_action_table(bool status);
    //! Sets the comment to the string provided by the user
    /*!
     \param comment to set in the component
     */
    virtual void set_comment(std::string comment);
    ///@private
    virtual void set_user_name(std::string user_name);
    ///@private
    virtual void set_date_mod(std::string date);
    //! Sets the model name to the string provided by the user
    /*!
    \param model_name to set in the component
    */
    virtual void set_model_name(std::string model_name);
    //! Sets the reference identification to the string provided by the user
    /*!
    \param ref_id to set in the component
    */
    virtual void set_ref_id(std::string ref_id);
    //! Sets the material name to the string provided by the user
    /*!
    \param material name to set in the component
    */
    virtual void set_material_name(std::string material);
    //! Sets the sequence number to the integer provided by the user
    /*!
    \param seq_num to set in the component
    */
    virtual void set_sequence_number(int seq_num)
    {
        _sequence_number = seq_num;
        _is_modified = true;
    }
    ///@private
    virtual void set_group_index(int index)
    {
        _group_index = index;
    }
    ///@private
    virtual void set_com_index(int index)
    {
        _com_index = index;
    }
    ///@private
    virtual void set_oper_index(int index)
    {
        _oper_index = index;
    }
    ///@private
    virtual std::string get_key_as_string() const;
    //! Returns the name of the component
    virtual std::string get_name() const
    {
        return _name;
    }
    //! Returns the prefix name of the component
    virtual std::string get_name_prefix() const
    {
        return _name_prefix;
    }
    //! Returns the complete name (prefix + name) of the component
    virtual std::string get_complete_name_spec() const
    {
        return _name_prefix + " " + _name;
    }
    ///@private
    virtual std::string get_class_sort_key() const
    {
        return _class_sort_key;
    }
    //! Returns the component type
    virtual std::string get_comp_type() const
    {
        return _type_string;
    }
    //! Returns the comment
    virtual std::string get_comment() const
    {
        return _comment;
    }
    ///@private
    virtual std::string get_user_name() const
    {
        return _user_name;
    }
    ///@private
    virtual std::string get_date_mod() const
    {
        return _date_modified;
    }
    //! Returns the model name of the component
    virtual std::string get_model_name() const
    {
        return _model_name;
    }
    //! Returns the reference identification of the component
    virtual std::string get_ref_id() const
    {
        return _ref_id;
    }
    //! Returns the material name of the component
    virtual std::string get_material_name() const
    {
        return _material_name;
    }
    //! Returns the keywords of the component
    /*!
     * The keywords are supplied as a string seperate by spaces.
     */
    virtual std::vector<std::string> get_keywords() const
    {
        return _keywords;
    }
    //! Returns the position (x,y) of the component in the Igrafix diagram
    virtual std::vector<float> get_position() const
    {
        return _item_position;
    }
    //! Sets the component position to the given position
    /*!
     /\param position vector of the x and y position to which the component is
     set.
     */
    virtual void set_position(const std::vector<float> &position)
    {
        _item_position = position;
        _is_modified = true;
    }
    //! returns the class name of the component
    virtual std::string get_class_name()
    {
        return _class_name;
    }
    //! returns true when the component contains the given property
    /*!
     \param property_description of the property to check.
     */
    virtual bool contains_property(const std::string &property_description);
    //! Returns a reference to a property of the component
    /*!
    * get_property() returns a reference to a wanda_property object that
    represents the
    * requested property of the wanda_component.
    \param property_description of the requested property
    */
    virtual wanda_property &get_property(std::string property_description);
    ///@private
    virtual void add_message(std::string message_in, char type, float time);
    //! Returns a vector with all the messages of the component
    virtual std::vector<std::string> get_messages();
    //! Returns a vector with all the simulation time steps at which the messages
    //! occured.
    virtual std::vector<float> get_message_time();
    //! Returns a vector with all the type of the messages
    /*!
     *get_message_type() returns the message types of the wanda component
     *Type can be I, for information, W for Warning and E for error.
     */
    virtual std::vector<char> get_message_type();
    //! Validates the item for connections and properties
    // virtual std::vector<std::string> validate() = 0;
    //@private
    std::vector<std::string> validate_input(int view_mask_model);
    //! Returns a vector of wanda_message class containing all information on the
    //! messages.
    /*!
     *get_all_messages() , returns a vector of the Wanda message class of all the
     *messages of the component The Wanda message class, contains the message, the
     *time and the type of message.
     */
    virtual std::vector<wanda_message> get_all_messages() const
    {
        return _messages;
    }
    //! Returns pointers to all properties.
    /*!
     * get_all_properties() returns a std::vector<wanda_property*>
     * object that contains pointers to all the properties in the wanda_component.
     */
    std::vector<wanda_property *> get_all_properties();
    //! Adds the data given in the template data to the component for the
    //! properties which exist in the component
    /*!
     \param data is unordered map with the data to add to the component.
     */
    virtual void add_data_from_template(std::unordered_map<std::string, wanda_prop_template> data);
    ///@private
    virtual std::string get_default_mask() const
    {
        return _default_mask;
    }
    ///@private
    virtual void set_default_mask(std::string def_mask)
    {
        _default_mask = def_mask;
    }
    ///@private
    std::string get_convert2component() const
    {
        return _convert2comp;
    }
    ///@private
    virtual void set_convert2component(std::string conv2comp)
    {
        _convert2comp = conv2comp;
    }
    //! returns the type name of the component or node
    virtual std::string get_type_name() const
    {
        return _type_name;
    }
    ///@private
    // returns hash of the object string for object identification (used in c_wanda_engine)
    virtual std::size_t wnd_get_hash_item()
    {
        return _object_hash_item;
    }
    ///@private
    virtual int get_number_of_species() const
    {
        return *num_of_species;
    }
    ///@private
    virtual void set_number_of_species(int *num_species)
    {
        num_of_species = num_species;
    }
    //! copies all input from the input component or node to this component
    virtual void copy_input(wanda_item &item);
    virtual int get_number_of_outputs();
    //! adds diagram line information to the component
    virtual void add_lines_info(wanda_diagram_lines *line)
    {
        lines_info.push_back(line);
    }
    virtual std::vector<wanda_diagram_lines*> get_all_line_info()
    {
        return lines_info;
    }

};

#endif
