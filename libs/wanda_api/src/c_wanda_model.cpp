// c interface functions
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <wandamodel.h>

static std::string wandamodel_Id_string("WandaModel Object");
static std::string item_Id_string("WandaItem Object");
static std::string prop_Id_string("WandaProperty Object");
static std::string table_Id_string("WandaTable Object");
static std::string component_Id_string("WandaComponent Object");
static std::string node_Id_string("WandaNode Object");
static std::string signal_line_Id_string("WandaSignalLine Object");
static std::size_t _wandamodel_Id_hash = std::hash<std::string>{}(wandamodel_Id_string);
static std::size_t _item_Id_hash = std::hash<std::string>{}(item_Id_string);
static std::size_t _component_Id_hash = std::hash<std::string>{}(component_Id_string);
static std::size_t _node_Id_hash = std::hash<std::string>{}(node_Id_string);
static std::size_t _singal_line_Id_hash = std::hash<std::string>{}(signal_line_Id_string);
static std::size_t _prop_Id_hash = std::hash<std::string>{}(prop_Id_string);
static std::size_t _table_Id_hash = std::hash<std::string>{}(table_Id_string);

static std::string wnd_model_error_message = "no error";

wanda_model *cast_to_wanda_model(void *void_pointer)
{
    auto model = static_cast<wanda_model *>(void_pointer);
    if (model->wnd_get_hash() != _wandamodel_Id_hash)
        throw std::exception("Invalid pointer cast!");
    return model;
}

wanda_item *cast_to_wanda_item(void *item_pointer)
{
    auto item = static_cast<wanda_item *>(item_pointer);
    if (item->wnd_get_hash_item() != _item_Id_hash)
    {
        throw std::exception("Invalid pointer cast!");
    }
    return item;
}

wanda_component *cast_to_wanda_component(void *void_pointer)
{
    auto component = static_cast<wanda_component *>(void_pointer);
    if (component->wnd_get_hash() != _component_Id_hash)
        throw std::exception("Invalid pointer cast!");
    return component;
}

wanda_node *cast_to_wanda_node(void *void_pointer)
{
    auto node = static_cast<wanda_node *>(void_pointer);
    if (node->wnd_get_hash() != _node_Id_hash)
        throw std::exception("Invalid pointer cast!");
    return node;
}

wanda_sig_line *cast_to_wanda_sig_line(void *void_pointer)
{
    auto sig_line = static_cast<wanda_sig_line *>(void_pointer);
    if (sig_line->wnd_get_hash() != _singal_line_Id_hash)
        throw std::exception("Invalid pointer cast!");
    return sig_line;
}

wanda_property *cast_to_wanda_property(void *void_pointer)
{
    auto prop = static_cast<wanda_property *>(void_pointer);
    if (prop->wnd_get_hash() != _prop_Id_hash)
        throw std::exception("Invalid pointer cast!");
    return prop;
}

wanda_table *cast_to_wanda_table(void *void_pointer)
{
    auto table = static_cast<wanda_table *>(void_pointer);
    if (table->wnd_get_hash() != _table_Id_hash)
        throw std::exception("Invalid pointer cast!");
    return table;
}

extern "C" __declspec(dllexport) const char *wnd_get_last_error()
{
#ifdef DEBUG
    std::cerr << "Last Error message: " << wnd_model_error_message << '\n';
#endif
    return wnd_model_error_message.c_str();
}

extern "C" __declspec(dllexport) void *wnd_load_wanda_model(const char *wanda_case, const char *wanda_dir)
{
    try
    {
        std::string wanda_case2 = std::string(wanda_case);
        std::string wanda_dir2 = std::string(wanda_dir);
        wanda_model *w_model = new wanda_model(wanda_case2, wanda_dir2);
        return static_cast<void *>(w_model);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr; // is returned to C implementation, works for most
                        // compilers/libs....
    }
}

extern "C" __declspec(dllexport) int wnd_close_wanda_model(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        model->close();
        delete model;
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_save_model_input(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        model->save_model_input();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_reload_model_input(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        model->reload_input();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_reload_model_output(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        model->reload_output();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_num_time_steps(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        return model->get_num_time_steps();
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_num_components(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        return static_cast<int>(model->get_all_components().size());
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_num_nodes(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        return static_cast<int>(model->get_all_nodes().size());
        ;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_num_pipes(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        return static_cast<int>(model->get_all_pipes().size());
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_all_component_handles(void *model_handle, void **buffer, const int size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto allcomp = model->get_all_components();
        if (size < allcomp.size())
            throw std::invalid_argument("buffer is too small, should be " + std::to_string(allcomp.size()) + " bytes");
        for (int i = 0; i < allcomp.size(); i++)
        {
            buffer[i] = static_cast<void *>(allcomp[i]);
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_all_node_handles(void *model_handle, void **buffer, const int size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto all_nodes = model->get_all_nodes();
        if (size < all_nodes.size())
            throw std::invalid_argument("buffer is too small");
        for (int i = 0; i < all_nodes.size(); i++)
        {
            buffer[i] = static_cast<void *>(all_nodes[i]);
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_all_pipe_handles(void *model_handle, void **buffer, const int size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto allpipes = model->get_all_pipes();
        if (size < allpipes.size())
            throw std::invalid_argument("buffer is too small");
        for (int i = 0; i < allpipes.size(); i++)
        {
            buffer[i] = static_cast<void *>(allpipes[i]);
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) void *wnd_get_component_handle(void *model_handle, const char *component_name)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto &component = model->get_component(std::string(component_name));
        return static_cast<void *>(&component);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) void *wnd_get_node_handle(void *model_handle, const char *node_name)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto &handle = model->get_node(std::string(node_name));
        return static_cast<void *>(&handle);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) void *wnd_get_model_property_handle(void *model_handle, const char *property_name)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto &handle = model->get_property(std::string(property_name));
        return static_cast<void *>(&handle);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) void *wnd_get_item_property_handle(void *item_handle, const char *property_name)
{
    try
    {

        auto component = cast_to_wanda_item(item_handle);

        auto &handle = component->get_property(std::string(property_name));
        return static_cast<void *>(&handle);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) int wnd_get_property_value(void *property_handle, float *pvalue)
{
    try
    {
        auto property = cast_to_wanda_property(property_handle);
        *pvalue = property->get_scalar_float();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_set_property_value(void *property_handle, const float value)
{
    try
    {
        auto property = cast_to_wanda_property(property_handle);
        property->set_scalar(value);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_property_value_str(void *property_handle, char *buffer,
                                                                const size_t buffersize)
{
    try
    {
        auto property = cast_to_wanda_property(property_handle);
        std::string value = property->get_scalar_str();
        if (buffersize < value.length() + 1) //+1 to leave room for \0 character
            throw std::runtime_error("buffersize to small");
        strncpy_s(buffer, buffersize, value.c_str(), value.length());
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_size_property_value_str(void *property_handle)
{
    try
    {
        auto property = cast_to_wanda_property(property_handle);
        std::string value = property->get_scalar_str();
        return static_cast<int>(value.size());
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_set_property_value_str(void *property_handle, const char *string)
{
    try
    {
        const std::string value(string);
        auto property = cast_to_wanda_property(property_handle);
        property->set_scalar(value);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_new_wanda_case(void *model_handle, const char *new_case_name)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        model->new_wanda_case(std::string(new_case_name));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) void *wnd_create_new_wanda_case(const char *new_case_name, const char *wandadir)
{
    try
    {
        return static_cast<void *>(create_new_wanda_case(std::string(new_case_name), std::string(wandadir)));
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) void *wnd_add_component(void *model_handle, const char *component_type_name,
                                                         const float x_pos, const float y_pos)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto &p = model->add_component(std::string(component_type_name), {x_pos, y_pos});
        return static_cast<void *>(&p);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) void *wnd_add_node(void *model_handle, const char *node_type_name, const float x_pos,
                                                    const float y_pos)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto &p = model->add_node(std::string(node_type_name), {x_pos, y_pos});
        return static_cast<void *>(&p);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) int wnd_delete_component(void *model_handle, void *component_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto component = cast_to_wanda_component(component_handle);
        model->delete_component(*component);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_delete_node(void *model_handle, void *node_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto node = cast_to_wanda_node(node_handle);
        model->delete_node(*node);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_delete_signal_line(void *model_handle, void *sigline_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto signal_line = cast_to_wanda_sig_line(sigline_handle);
        model->delete_sig_line(*signal_line);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) void *wnd_connect_components(void *model_handle, void *component1,
                                                              const int connection_point1, void *component2,
                                                              const int connection_point2)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto comp1 = cast_to_wanda_component(component1);
        auto comp2 = cast_to_wanda_component(component2);
        auto &item_handle = model->connect(*comp1, connection_point1, *comp2, connection_point2);
        return static_cast<void *>(&item_handle);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) int wnd_connect_component_to_node(void *model_handle, void *component1,
                                                                   const int connection_point1, void *node)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto comp1 = cast_to_wanda_component(component1);
        auto node_handle = cast_to_wanda_node(node);
        model->connect(*comp1, connection_point1, *node_handle);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_disconnect_component(void *model_handle, void *component_handle,
                                                              const int connection_point)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto comp1 = cast_to_wanda_component(component_handle);
        model->disconnect(*comp1, connection_point);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_set_item_position(void *item_handle, const float x_pos, const float y_pos)
{
    try
    {
        auto item = cast_to_wanda_item(item_handle);
        item->set_position({x_pos, y_pos});
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_item_position(void *item_handle, float *x_pos, float *y_pos)
{
    try
    {
        auto item = cast_to_wanda_item(item_handle);
        auto position = item->get_position();
        *x_pos = position[0];
        *y_pos = position[1];
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_item_name_spec(void *item_handle, char *buffer, const size_t buffersize)
{
    try
    {
        auto item = cast_to_wanda_item(item_handle);
        std::string item_name = item->get_complete_name_spec();
        if (buffersize < item_name.length() + 1) //+1 to leave room for \0 character
            throw std::runtime_error("buffersize to small");
        strncpy_s(buffer, buffersize, item_name.c_str(), item_name.length());
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_length_item_name_spec(void *item_handle)
{
    try
    {
        auto item = cast_to_wanda_item(item_handle);
        std::string item_name = item->get_complete_name_spec();
        return static_cast<int>(item_name.size());
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_item_name(void *item_handle, char *buffer, const size_t buffersize)
{
    try
    {
        auto item = cast_to_wanda_item(item_handle);
        std::string item_name = item->get_name();
        if (buffersize < item_name.length() + 1) //+1 to leave room for \0 character
            throw std::runtime_error("buffersize to small");
        strncpy_s(buffer, buffersize, item_name.c_str(), item_name.length());
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_item_classname(void *item_handle, char *buffer, const size_t buffersize)
{
    try
    {
        auto item = cast_to_wanda_item(item_handle);
        std::string item_class = item->get_class_name();
        if (buffersize < item_class.length() + 1) //+1 to leave room for \0 character
            throw std::runtime_error("buffersize to small");
        strncpy_s(buffer, buffersize, item_class.c_str(), item_class.length());
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_item_type(void *item_handle)
{
    try
    {
        auto item = cast_to_wanda_item(item_handle);
        auto type = item->get_item_type();
        switch (type)
        {
        case wanda_type::physical:
            return 1;
        case wanda_type::control:
            return 2;
        case wanda_type::node:
            return 3;
        case wanda_type::signal_line:
            return 4;
        default:
            return -1;
        }
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_num_connectpoints(void *component_handle)
{
    try
    {
        auto handle = cast_to_wanda_component(component_handle);
        return handle->get_number_of_connnect_points();
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_flipped_status(void *component_handle)
{
    try
    {
        auto handle = cast_to_wanda_component(component_handle);
        return handle->get_is_flipped() ? -1 : 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_disused_status(void *component_handle)
{
    try
    {
        auto handle = cast_to_wanda_component(component_handle);
        return handle->is_disused() ? 1 : 0;
        ;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_run_steady(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        model->run_steady();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_run_unsteady(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        model->run_unsteady();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) void *wnd_split_pipe(void *model_handle, void *comp_handle, const float loc)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto comp = static_cast<wanda_component *>(comp_handle);
        if (comp->wnd_get_hash() != _item_Id_hash)
            throw std::exception("Invalid pointer cast: comp_handle");
        auto &node = model->split_pipe(*comp, loc);
        return static_cast<void *>(&node);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) int wnd_get_number_results_val_model(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        size_t size = 0;
        auto res = model->validate_model_input();
        for (auto &item : res)
        {
            size += item.second.size();
        }
        return size;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return 0;
    }
}

extern "C" __declspec(dllexport) int wnd_validate_model_input(void *model_handle, char **comps, const int comps_size,
                                                              char **props, const int props_size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto res = model->validate_model_input();
        size_t size = 0;
        for (auto &item : res)
        {
            size += item.second.size();
        }

        if (comps_size < size * (sizeof(char) * (8 + 1 + 16)))
        {
            throw std::invalid_argument("buffer comp_list is too small, should be " +
                                        std::to_string(res.size() * sizeof(char) * (8 + 1 + 16)) + " bytes");
        }
        if (props_size < size * (sizeof(char) * 30))
        {
            throw std::invalid_argument("buffer prop_list is too small, should be " +
                                        std::to_string(res.size() * sizeof(char) * 30) + " bytes");
        }
        int i = 0;
        for (auto item : res)
        {
            for (auto &prop : item.second)
            {
                strcpy_s(comps[i], item.first.size() + 1, item.first.c_str());
                strcpy_s(props[i], prop.size() + 1, prop.c_str());
                i++;
            }
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_number_results_check_con_model(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        size_t size = 0;
        auto res = model->validate_connectivity();
        for (auto &item : res)
        {
            size += item.second.size();
        }
        return size;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_check_connectivity(void *model_handle, char **comps,
                                                            const std::size_t comp_size, int *con_points,
                                                            const std::size_t cpoints_size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto res = model->validate_connectivity();
        if (comp_size < res.size() * (sizeof(char) * (8 + 1 + 16)))
        {
            throw std::invalid_argument("buffer comp_list is too small, should be " +
                                        std::to_string(res.size() * sizeof(char) * (8 + 1 + 16)) + " bytes");
        }
        if (cpoints_size < res.size() * (sizeof(int)))
        {
            throw std::invalid_argument("buffer con_points is too small, should be " +
                                        std::to_string(res.size() * sizeof(int)) + " bytes");
        }
        int i = 0;
        for (auto item : res)
        {
            for (auto &con_point : item.second)
            {
                strcpy_s(comps[i], item.first.size() + 1, item.first.c_str());
                con_points[i] = con_point;
                i++;
            }
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_extremes_max_pipe(void *prop_handle, float *buffer,
                                                               const size_t buffersize)
{
    try
    {
        auto property = cast_to_wanda_property(prop_handle);
        auto maximum = property->get_extr_max_pipe();
        if (maximum.size() > buffersize)
            throw std::invalid_argument("Buffer is too small should be " + std::to_string(maximum.size()));

        memcpy_s(buffer, buffersize * sizeof(float), maximum.data(), maximum.size() * sizeof(float));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_extremes_tmax_pipe(void *prop_handle, float *buffer,
                                                                const size_t buffersize)
{
    try
    {
        auto property = cast_to_wanda_property(prop_handle);
        auto result = property->get_extr_tmax_pipe();
        if ((result.size() * sizeof(float)) > buffersize)
            throw std::invalid_argument("Buffer is too small, should be " + std::to_string(result.size()));
        memcpy_s(buffer, buffersize, result.data(), result.size() * sizeof(float));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_extremes_tmin_pipe(void *prop_handle, float *buffer,
                                                                const size_t buffersize)
{
    try
    {
        auto property = cast_to_wanda_property(prop_handle);
        auto result = property->get_extr_tmin_pipe();
        if ((result.size()) > buffersize)
            throw std::invalid_argument("Buffer is too small, should be " + std::to_string(result.size()));
        memcpy_s(buffer, buffersize * sizeof(float), result.data(), result.size() * sizeof(float));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_extremes_min_pipe(void *prop_handle, float *buffer,
                                                               const size_t buffersize)
{
    try
    {
        auto property = cast_to_wanda_property(prop_handle);
        auto result = property->get_extr_min_pipe();
        if ((result.size()) > buffersize)
            throw std::invalid_argument("Buffer is too small, should be " + std::to_string(result.size()));
        memcpy_s(buffer, buffersize * sizeof(float), result.data(), result.size() * sizeof(float));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_num_elements(void *prop_handle)
{
    try
    {
        auto property = cast_to_wanda_property(prop_handle);
        return property->get_number_of_elements();
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_extremes_min(void *prop_handle, float *result)
{
    try
    {
        auto property = cast_to_wanda_property(prop_handle);
        *result = property->get_extr_min();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_extremes_max(void *prop_handle, float *result)
{
    try
    {
        auto property = cast_to_wanda_property(prop_handle);
        *result = property->get_extr_max();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_extremes_tmin(void *prop_handle, float *result)
{
    try
    {
        auto property = cast_to_wanda_property(prop_handle);
        *result = property->get_extr_tmin();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_extremes_tmax(void *prop_handle, float *result)
{
    try
    {
        auto property = cast_to_wanda_property(prop_handle);
        *result = property->get_extr_tmin();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) void *wnd_get_connected_node(void *comp_handle, int connectionpoint)
{
    try
    {
        auto component = cast_to_wanda_component(comp_handle);
        auto &handle = component->get_connected_node(connectionpoint);
        return static_cast<void *>(&handle);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) int wnd_get_connected_components(void *node_handle, void **buffer,
                                                                  const size_t buffersize)
{
    try
    {
        auto node = cast_to_wanda_node(node_handle);
        auto connected_comps = node->get_connected_components();
        if (buffersize < connected_comps.size())
            throw std::invalid_argument("buffer is too small should be size " + std::to_string(connected_comps.size()));
        for (int i = 0; i < connected_comps.size(); i++)
        {
            buffer[i] = static_cast<void *>(connected_comps[i]);
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_num_connected_components(void *node_handle)
{
    try
    {
        auto node = cast_to_wanda_node(node_handle);
        return (node->get_connected_components()).size();
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_contains_property(void *item_handle, char *property_name)
{
    try
    {
        auto item = cast_to_wanda_item(item_handle);
        if (item->contains_property(std::string(property_name)))
            return 0; // true
        return 1;     // false
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_series(void *property_handle, float *buffer, const size_t buffersize)
{
    try
    {
        auto property = cast_to_wanda_property(property_handle);
        auto result = property->get_series();
        if ((result.size() * sizeof(float)) > buffersize)
            throw std::invalid_argument("Buffer is too small");
        memcpy_s(buffer, buffersize, result.data(), result.size() * sizeof(float));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_time_series_pipe(void *property_handle, int element, float *buffer,
                                                              const size_t buffersize)
{
    try
    {
        auto property = cast_to_wanda_property(property_handle);
        auto result = property->get_series(element);
        if (result.size() * sizeof(float) > buffersize)
            throw std::invalid_argument("Buffer is too small");
        memcpy_s(buffer, buffersize, result.data(), result.size() * sizeof(float));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_series_pipe(void *property_handle, float *buffer, const size_t buffersize)
{
    try
    {
        const auto property = cast_to_wanda_property(property_handle);
        const size_t num_elements = property->get_number_of_elements();
        const auto result = property->get_series_pipe();
        const size_t num_timesteps = result[0].size();
        if ((num_elements + std::size_t{1}) * num_timesteps * sizeof(float) > buffersize)
            throw std::invalid_argument("Buffer is too small");

        for (int i = 0; i <= num_elements; i++)
        {
            for (int j = 0; j <= num_timesteps - 1; j++)
            {
                buffer[i * num_timesteps + j] = result[i][j];
            }
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_has_series(void *property_handle)
{
    try
    {
        auto property = cast_to_wanda_property(property_handle);
        return property->has_series() ? 0 : 1;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();

        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_merge_pipes(void *model, void *pipe1, void *pipe2, int option)
{
    try
    {
        auto comp_1 = cast_to_wanda_component(pipe1);
        auto comp_2 = cast_to_wanda_component(pipe2);
        auto model_ = cast_to_wanda_model(model);
        model_->merge_pipes(*comp_1, *comp_2, option);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();

        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_size_of_route(void *model_handle, const char *keyword)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        std::vector<int> direction;
        auto route = model->get_route(keyword, direction);
        return static_cast<int>(route.size());
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_route(void *model_handle, const char *keyword, void **components, int *dir,
                                                   const size_t size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        std::vector<int> direction;
        auto route = model->get_route(keyword, direction);
        if (size < route.size())
            throw std::invalid_argument("buffer is too small should be " + std::to_string(route.size()));
        for (int i = 0; i < route.size(); i++)
        {
            components[i] = static_cast<void *>(route[i]);
            dir[i] = direction[i];
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_size_of_phys_comp_type(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto types_list = model->get_possible_phys_comp_type();
        return static_cast<int>(types_list.size());
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_possible_phys_comp_type(void *model_handle, char **types,
                                                                     const size_t size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto types_list = model->get_possible_phys_comp_type();
        if (size < types_list.size())
            throw std::invalid_argument("wnd_get_possible_phys_comp_type buffer is too small should be " +
                                        std::to_string(types_list.size()));
        for (int i = 0; i < size; i++)
        {
            strncpy_s(types[i], 48, types_list[i].c_str(), types_list[i].length());
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_size_all_keywords(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto types_list = model->get_all_keywords();
        return static_cast<int>(types_list.size());
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_all_keywords(void *model_handle, char **types,
                                                                     const size_t size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto types_list = model->get_all_keywords();
        if (size < types_list.size())
            throw std::invalid_argument("wnd_get_all_keywords buffer is too small should be " +
                                        std::to_string(types_list.size()));
        for (int i = 0; i < size; i++)
        {
            strncpy_s(types[i], 48, types_list[i].c_str(), types_list[i].length());
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}


extern "C" __declspec(dllexport) int wnd_get_size_of_ctrl_comp_type(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto types_list = model->get_possible_ctrl_comp_type();
        return types_list.size();
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_possible_ctrl_comp_type(void *model_handle, char **types,
                                                                     const size_t size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto types_list = model->get_possible_ctrl_comp_type();
        if (size < types_list.size())
            throw std::invalid_argument("wnd_get_possible_ctrl_comp_type buffer is too small should be " +
                                        std::to_string(types_list.size()));
        for (int i = 0; i < size; i++)
        {
            strncpy_s(types[i], 48, types_list[i].c_str(), types_list[i].length());
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_size_of_node_type(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto types_list = model->get_possible_node_type();
        return types_list.size();
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_possible_node_type(void *model_handle, char **types, const size_t size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto types_list = model->get_possible_node_type();
        if (size < types_list.size())
            throw std::invalid_argument("wnd_get_possible_node_type buffer is too small should be " +
                                        std::to_string(types_list.size()));
        for (int i = 0; i < size; i++)
        {
            strncpy_s(types[i], 48, types_list[i].c_str(), types_list[i].length());
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_switch_to_transient_mode(void *model_pointer)
{
    try
    {
        auto model = cast_to_wanda_model(model_pointer);
        model->switch_to_transient_mode();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_switch_to_engineering_mode(void *model_pointer)
{
    try
    {
        auto model = cast_to_wanda_model(model_pointer);
        model->switch_to_engineering_mode();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_change_component_type(void *model_pointer, const char *comp_name,
                                                               const char *type)
{
    try
    {
        auto model = cast_to_wanda_model(model_pointer);
        model->change_comp_type(comp_name, type);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_change_node_type(void *model_pointer, const char *node_name, const char *type)
{
    try
    {
        auto model = cast_to_wanda_model(model_pointer);
        model->change_node_type(node_name, type);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_switch_to_SI_unit(void *model_pointer)
{
    try
    {
        auto model = cast_to_wanda_model(model_pointer);
        model->switch_to_unit_SI();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_switch_to_user_unit(void *model_pointer)
{
    try
    {
        auto model = cast_to_wanda_model(model_pointer);
        model->switch_to_unit_SI();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) void *wnd_get_signal_line(void *model_pointer, const char *name)
{
    try
    {
        auto model = cast_to_wanda_model(model_pointer);
        auto &sig_line = model->get_signal_line(name);
        return static_cast<void *>(&sig_line);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) int wnd_has_keyword(void *item_point, const char *keyword)
{
    try
    {
        auto item = cast_to_wanda_item(item_point);
        return item->has_keyword(keyword) ? 1 : 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_add_keyword(void *item_pointer, const char *keyword)
{
    try
    {
        auto item = cast_to_wanda_item(item_pointer);
        item->add_keyword(keyword);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_remove_keyword(void *item_pointer, const char *keyword)
{
    try
    {
        auto item = cast_to_wanda_item(item_pointer);
        item->remove_keyword(keyword);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_set_action_table(void *item, const int status)
{
    try
    {
        auto comp_point = cast_to_wanda_item(item);
        comp_point->set_use_action_table(status == 1);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_is_action_table_used(void *item)
{
    try
    {
        auto comp_point = cast_to_wanda_item(item);
        bool used = comp_point->is_action_table_used();
        return used ? 1 : 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_connected_signal_lines(void *item_handel, const int con_point,
                                                                    const int input, void **buffer,
                                                                    const size_t buffersize)
{
    try
    {
        auto item = cast_to_wanda_component(item_handel);
        auto connected_sig_lines = item->get_connected_sigline(con_point, input == 1);
        if (buffersize < (connected_sig_lines.size()))
            throw std::invalid_argument("buffer is too small should be " + std::to_string(connected_sig_lines.size()));
        for (int i = 0; i < connected_sig_lines.size(); i++)
        {
            buffer[i] = static_cast<void *>(connected_sig_lines[i]);
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_number_of_connected_signal_lines(void *item_handel, const int con_point,
                                                                              const int input)
{
    try
    {
        auto item = cast_to_wanda_component(item_handel);
        auto connected_sig_lines = item->get_connected_sigline(con_point, input == 1);
        return static_cast<int>(connected_sig_lines.size());
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_is_pipe(void *comp_handle)
{
    try
    {
        auto comp = cast_to_wanda_component(comp_handle);
        return comp->is_pipe() ? 1 : 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) void *wnd_get_input_component(void *signal_line_handle)
{
    try
    {
        auto sig_line = cast_to_wanda_sig_line(signal_line_handle);
        return static_cast<void *>(sig_line->get_input_component());
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) void *wnd_get_output_component(void *signal_line_handle)
{
    try
    {
        auto sig_line = cast_to_wanda_sig_line(signal_line_handle);
        return static_cast<void *>(sig_line->get_output_component());
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) int wnd_get_unit_factor(void *prop_handle, float *factor)
{
    try
    {
        auto prop = cast_to_wanda_property(prop_handle);
        *factor = prop->get_unit_factor();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_float_column(void *table_handle, const char *description, float *values,
                                                          const size_t size)
{
    try
    {
        auto table = cast_to_wanda_table(table_handle);
        auto result = table->get_float_column(description);

        if ((result.size()) > size)
            throw std::invalid_argument("Buffer is too small should be " + std::to_string(size));
        memcpy_s(values, size * sizeof(float), result.data(), result.size() * sizeof(float));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_table_size(void *table_handle, const char *description)
{
    try
    {
        auto table = cast_to_wanda_table(table_handle);
        size_t size = 0;
        if (table->is_string_column(description))
        {
            auto result = table->get_string_column(description);
            size = result.size();
        }
        else
        {
            auto result = table->get_float_column(description);
            size = result.size();
        }
        return size;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_set_float_column(void *table_handle, const char *description, float *values,
                                                          const size_t size)
{
    try
    {
        auto table = cast_to_wanda_table(table_handle);
        std::vector<float> tab_vals{values, values + size};
        table->set_float_column(description, tab_vals);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) void *wnd_get_property_table(void *prop_handle)
{
    try
    {
        auto prop = cast_to_wanda_property(prop_handle);
        auto &table = prop->get_table();
        return static_cast<void *>(&table);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return nullptr;
    }
}

extern "C" __declspec(dllexport) int wnd_load_data_from_template_to_model(void *model_pointer,
                                                                          const char *template_file)
{
    try
    {
        auto model = cast_to_wanda_model(model_pointer);
        model->add_data_from_template_file(template_file);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
    return 0;
}

extern "C" __declspec(dllexport) int wnd_get_time_steps(void *model_pointer, float *time_steps, const size_t size)
{
    try
    {
        auto model = cast_to_wanda_model(model_pointer);
        auto time_steps_vec = model->get_time_steps();
        if (time_steps_vec.size() > size)
        {
            throw std::invalid_argument("Buffer is too small should be " + std::to_string(time_steps_vec.size()));
        }
        memcpy_s(time_steps, size * sizeof(float), time_steps_vec.data(), time_steps_vec.size() * sizeof(float));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_components_with_keyword(void *model_handle, const char *keyword,
                                                                     void **handles, const size_t size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto comps = model->get_components_with_keyword(keyword);
        if (comps.size() > size)
        {
            throw std::invalid_argument("Buffer size to small should be " + std::to_string(comps.size()));
        }
        for (int i = 0; i < comps.size(); i++)
        {
            handles[i] = static_cast<void *>(comps[i]);
        }
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
    return 0;
}

extern "C" __declspec(dllexport) int wnd_get_number_of_components_with_keyword(void *model_handle, char *keyword)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto comps = model->get_components_with_keyword(keyword);
        return comps.size();
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_number_of_model_properties(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto props = model->get_all_properties();
        return props.size();
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_model_properties(void *model_handle, void **properties, const size_t size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto props = model->get_all_properties();
        if (props.size() > size)
        {
            throw std::invalid_argument("Buffer size to small should be " + std::to_string(props.size()));
        }
        for (int i = 0; i < props.size(); i++)
        {
            properties[i] = static_cast<void *>(props[i]);
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_max_string_size_properties_model(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto props = model->get_all_properties_string();
        size_t max_size = 0;
        for (auto &prop : props)
        {
            if (prop.size() > max_size)
            {
                max_size = prop.size();
            }
        }
        return max_size;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_model_properties_string(void *model_handle, char *properties,
                                                                     const size_t size)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        auto props = model->get_all_properties_string();
        size_t max_size = 0;
        for (auto &prop : props)
        {
            if (prop.size() > max_size)
            {
                max_size = prop.size();
            }
        }
        if (props.size() > size * (max_size))
        {
            throw std::invalid_argument("Buffer size to small should be " + std::to_string(props.size() * (max_size)));
        }
        for (int i = 0; i < props.size(); i++)
        {
            for (int j = 0; j < props[i].size(); j++)
            {
                properties[i * max_size + j] = props[i][j];
            }
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_all_item_properties(void *item_handle, void **properties,
                                                                 const size_t size)
{
    try
    {
        auto item = cast_to_wanda_item(item_handle);
        auto props = item->get_all_properties();
        if (props.size() > size)
        {
            throw std::invalid_argument("Buffer size to small should be " + std::to_string(props.size()));
        }
        for (int i = 0; i < props.size(); i++)
        {
            properties[i] = static_cast<void *>(props[i]);
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_number_of_item_properties(void *item_handle)
{
    try
    {
        auto item = cast_to_wanda_item(item_handle);
        auto props = item->get_all_properties();
        return props.size();
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_size_item_property_string(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        std::string element = "Spec_descr";
        return model->get_element_size_def(element);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_all_item_properties_string(void *model_handle, void *item_handle,
                                                                        char *properties, int size)
{
    try
    {
        auto item = cast_to_wanda_item(item_handle);
        auto props = item->get_all_properties();
        int string_size = wnd_get_size_item_property_string(model_handle);
        if (props.size() * string_size > size)
        {
            throw std::invalid_argument("Buffer size to small should be " + std::to_string(props.size() * string_size));
        }
        for (int i = 0; i < props.size(); i++)
        {
            for (int j = 0; j < props[i]->get_description().size(); j++)
            {
                properties[i * string_size + j] = props[i]->get_description()[j];
            }
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_property_description(void *property_handle, char *description, int size)
{
    try
    {
        auto property = cast_to_wanda_property(property_handle);
        auto description_ = property->get_description();
        if (description_.size() > size)
        {
            throw std::invalid_argument("Buffer size to small should be " + std::to_string(description_.size()));
        }

        strncpy_s(description, size, description_.c_str(), description_.length());
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_table_descriptions(void *model_handle, void *table_handle,
                                                                char *descriptions, const size_t size)
{
    try
    {
        auto table = cast_to_wanda_table(table_handle);
        auto model = cast_to_wanda_model(model_handle);
        auto descriptions_ = table->get_descriptions();
        std::string element = "Table_descr";
        int string_size = model->get_element_size_wdi(element);
        if (descriptions_.size() * string_size > size)
        {
            throw std::invalid_argument("Buffer size to small should be " +
                                        std::to_string(descriptions_.size() * string_size));
        }
        for (int i = 0; i < descriptions_.size(); i++)
        {
            for (int j = 0; j < descriptions_[i].size(); j++)
            {
                descriptions[i * string_size + j] = descriptions_[i][j];
            }
        }
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_number_table_descriptions(void *table_handle)
{
    try
    {
        auto table = cast_to_wanda_table(table_handle);
        auto descriptions_ = table->get_descriptions();
        return descriptions_.size();
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_size_property_description(void *property_handle)
{
    try
    {
        auto property = cast_to_wanda_property(property_handle);
        auto description_ = property->get_description();
        return description_.size();
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_get_string_size_table_description(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        std::string element = "Table_descr";
        return model->get_element_size_wdi(element);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_component_exists(void *model_handle, char *name)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        std::string str_name = name;
        return model->component_exists(str_name) ? 1 : 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_node_exists(void *model_handle, char *name)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        std::string str_name = name;
        return model->node_exists(str_name) ? 1 : 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_reset_wdo_pointer(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        model->reset_wdo_pointer();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_resume_unsteady_until(void *model_handle, float end_time)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        model->resume_unsteady_until(end_time);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
       return -1;
    }
}


extern "C" __declspec(dllexport) int wnd_upgrade_model(void *model_handle)
{
    try
    {
        auto model = cast_to_wanda_model(model_handle);
        model->upgrade_model();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_model_error_message = e.what();
        return -1;
    }
}