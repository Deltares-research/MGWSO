
#include "wanda_engine.h"
#include <functional>

static std::string wnd_eng_error_message = "no error";

static std::string engine_Id_string("WandaEngine Object");
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
static std::size_t _engine_Id_hash = std::hash<std::string>{}(engine_Id_string);

extern "C" __declspec(dllexport) const char *wnd_eng_get_last_error()
{
#ifdef DEBUG
    std::cerr << "Last error message: " << wnd_eng_error_message << '\n';
#endif
    return wnd_eng_error_message.c_str();
}

extern "C" __declspec(dllexport) void *wnd_eng_get_instance(const char *wanda_bin)
{
    try
    {
        wanda_engine *w_engine = wanda_engine::get_instance(wanda_bin);
        return static_cast<void *>(w_engine);
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return nullptr; // is returned to C implementation, works for most
                        // compilers/libs.
    }
}

extern "C" __declspec(dllexport) int wnd_eng_initialize(void *engine, void *model)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");

        wanda_model *model1 = static_cast<wanda_model *>(model);
        if (model1->wnd_get_hash() != _wandamodel_Id_hash)
            throw std::exception("Invalid pointer cast!");
        engine1->initialize_engine(model1->get_case_path());
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_run_steady(void *engine)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        engine1->run_steady();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_run_time_step(void *engine)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        engine1->run_time_step();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_finish_unsteady(void *engine)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        engine1->finish_unsteady();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_close(void *engine)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        engine1->close_engine();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_get_value(void *engine, char *name, char *prop, double *result)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        *result = engine1->get_value(std::string(name), std::string(prop));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_get_value_comp(void *engine, void *comp, char *prop, double *result)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        wanda_component *component = static_cast<wanda_component *>(comp);
        if (component->wnd_get_hash() != _component_Id_hash)
            throw std::exception("Invalid pointer cast!");
        *result = engine1->get_value(*component, std::string(prop));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_set_value(void *engine, char *name, char *prop, double *value)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        auto component_name = std::string(name);
        auto property = std::string(prop);
        engine1->set_value(component_name, property, *value);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_set_value_comp(void *engine, void *comp, char *prop, double *value)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        wanda_component *component = static_cast<wanda_component *>(comp);
        if (component->wnd_get_hash() != _component_Id_hash)
            throw std::exception("Invalid pointer cast!");
        auto propertyname = std::string(prop);
        engine1->set_value(*component, propertyname, *value);
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_get_vector(void *engine, char *name, char *prop, double *values,
                                                        int buffersize)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        auto result = engine1->get_vector(std::string(name), std::string(prop));
        if (result.size() > buffersize)
            throw std::invalid_argument("Buffer is too small");
        memcpy_s(values, buffersize * sizeof(double), result.data(), result.size() * sizeof(double));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_get_vector_comp(void *engine, void *comp, char *prop, double *values,
                                                             int buffersize)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        wanda_component *component = static_cast<wanda_component *>(comp);
        if (component->wnd_get_hash() != _component_Id_hash)
            throw std::exception("Invalid pointer cast!");
        auto result = engine1->get_vector(*component, std::string(prop));
        if (result.size() > buffersize)
            throw std::invalid_argument("Buffer is too small");
        memcpy_s(values, buffersize * sizeof(double), result.data(), result.size() * sizeof(double));
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_get_start_time(void *engine, double *time)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        *time = engine1->get_start_time();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_get_end_time(void *engine, double *time)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        *time = engine1->get_end_time();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_get_current_time(void *engine, double *time)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        *time = engine1->get_current_time();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}

extern "C" __declspec(dllexport) int wnd_eng_get_time_step(void *engine, double *time)
{
    try
    {
        wanda_engine *engine1 = static_cast<wanda_engine *>(engine);
        if (engine1->wnd_get_hash() != _engine_Id_hash)
            throw std::exception("Invalid pointer cast!");
        *time = engine1->get_delta_t();
        return 0;
    }
    catch (std::exception &e)
    {
#ifdef DEBUG
        std::cerr << e.what() << std::endl;
#endif
        wnd_eng_error_message = e.what();
        return -1;
    }
}
