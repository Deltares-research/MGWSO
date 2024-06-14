#ifndef _WANDANODE_
#define _WANDANODE_

#include <string>
#include <unordered_map>
#include <wanda_item.h>

#ifdef WANDAMODEL_EXPORT
// #define WANDAMODEL_API __declspec(dllexport)
#define WANDAMODEL_API 

#else
#define WANDAMODEL_API __declspec(dllimport)
#endif

class wanda_def;
class wanda_component;

//!  Class which provides access to nodes.
/*!
The wanda node class can be used to read and adjust nodes in the model.
*/
class WANDAMODEL_API wanda_node : public wanda_item
{
  public:
    ///@private
    wanda_node();
    ///@private
    wanda_node(int, std::string, std::string, std::string, std::string, std::string type,
               wanda_def *component_definition);
    ///@private
    wanda_node(std::string class_sort_key, wanda_type type, std::string name_pre_fix, std::string type_name,
               std::string def_mask, std::string convert2comp, wanda_def *component_definition);
    //! Returns a vector of pointers to the components connected to the node
    std::vector<wanda_component *> get_connected_components() const;
    ///@private
    void connect(wanda_component &component);
    ///@private
    void disconnect(wanda_component &component);
    ///@private
    void disconnect();
    ///@private
    std::string get_core_quants() const
    {
        return _core_quantities[0];
    }
    ///@private
    std::string get_node_type() const
    {
        return _node_type;
    }
    ///@private
    // returns hash of the object string for object identification (used in c_wanda_engine)
    virtual std::size_t wnd_get_hash()
    {
        return _object_hash;
    }

  private:
    const static std::string _object_name;
    const std::size_t _object_hash;
    std::vector<wanda_component *> _connected_comps;
    std::string _node_type;
};

#endif
