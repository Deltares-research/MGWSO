
#include <functional>
// #include <lcencdec.h>
#include <Windows.h>
// #include <spdlog/spdlog.h>
#include <stdexcept>
#include <unordered_map>
#include <wanda_engine.h>

// wanda_engine *wanda_engine::_instance = nullptr;
// const std::string wanda_engine::_object_name = "WandaEngine Object";
wanda_engine::wanda_engine(const std::string &wanda_bin) : _wanda_bin(wanda_bin)
{
    SetDllDirectoryA(_wanda_bin.c_str());
    hGetProcIDDLL = LoadLibrary("WandaEngine_native64.dll");
    if (!hGetProcIDDLL)
    {
        throw std::runtime_error("Could not load WandaEngine_native64.dll or it's dependencies.");
    }
    // spdlog::info("WandaEngine_native64 DLL loaded, handle={}", fmt::ptr(hGetProcIDDLL));
    wnd_main_init = wanda_helper_functions::loadDLLfunction<int(const char *, size_t)>(hGetProcIDDLL, "WND_MAIN_INIT");
    wnd_main_final = wanda_helper_functions::loadDLLfunction<int()>(hGetProcIDDLL, "WND_MAIN_FINAL");
    wnd_load_data = wanda_helper_functions::loadDLLfunction<int()>(hGetProcIDDLL, "WND_LOAD_DATA");
    wnd_stdy_data_init = wanda_helper_functions::loadDLLfunction<int()>(hGetProcIDDLL, "WND_STEADY_DATA_INIT");
    wnd_stdy_comp_init = wanda_helper_functions::loadDLLfunction<int()>(hGetProcIDDLL, "WND_STEADY_COMP_INIT");
    wnd_stdy_compute = wanda_helper_functions::loadDLLfunction<int()>(hGetProcIDDLL, "WND_STEADY_COMPUTE");
    wnd_stdy_final = wanda_helper_functions::loadDLLfunction<int()>(hGetProcIDDLL, "WND_STEADY_FINAL");
    wnd_unstdy_init = wanda_helper_functions::loadDLLfunction<int()>(hGetProcIDDLL, "WND_UNSTEADY_INIT");
    wnd_unstdy_compute = wanda_helper_functions::loadDLLfunction<int(int *)>(hGetProcIDDLL, "WND_UNSTEADY_COMPUTE");
    wnd_unstdy_final = wanda_helper_functions::loadDLLfunction<int()>(hGetProcIDDLL, "WND_UNSTEADY_FINAL");
    wnd_get_current_time =
        wanda_helper_functions::loadDLLfunction<int(double *)>(hGetProcIDDLL, "WND_GET_CURRENT_TIME");
    wnd_get_delta_t = wanda_helper_functions::loadDLLfunction<int(double *)>(hGetProcIDDLL, "WND_GET_DELTA_T");
    wnd_get_start_time = wanda_helper_functions::loadDLLfunction<int(double *)>(hGetProcIDDLL, "WND_GET_START_TIME");
    wnd_get_end_time = wanda_helper_functions::loadDLLfunction<int(double *)>(hGetProcIDDLL, "WND_GET_END_TIME");
    wnd_get_component_handle = wanda_helper_functions::loadDLLfunction<int(const char *, const char *, size_t, size_t)>(
        hGetProcIDDLL, "WND_GETCOMPONENTHANDLE");
    wnd_get_property_handle = wanda_helper_functions::loadDLLfunction<int(const int *, const char *, size_t)>(
        hGetProcIDDLL, "WND_GETPROPERTYHANDLE");
    wnd_get_values = wanda_helper_functions::loadDLLfunction<int(const int *, const int *, double *, int *)>(
        hGetProcIDDLL, "WND_GETVALUES");
    wnd_set_values = wanda_helper_functions::loadDLLfunction<int(const int *, const int *, double *, int *)>(
        hGetProcIDDLL, "WND_SETVALUES");
    wnd_get_vector = wanda_helper_functions::loadDLLfunction<int(const int *, const int *, double *, int *)>(
        hGetProcIDDLL, "WND_GETVECTOR");
    wnd_get_values_by_name =
        wanda_helper_functions::loadDLLfunction<int(const char *, const char *, const char *, double *, int *, size_t,
                                                    size_t, size_t)>(hGetProcIDDLL, "WND_GETVALUESBYNAME");
    wnd_set_values_by_name =
        wanda_helper_functions::loadDLLfunction<int(const char *, const char *, const char *, double *, int *, size_t,
                                                    size_t, size_t)>(hGetProcIDDLL, "WND_SETVALUESBYNAME");
    wnd_get_vector_by_name =
        wanda_helper_functions::loadDLLfunction<int(const char *, const char *, const char *, double *, int *, size_t,
                                                    size_t, size_t)>(hGetProcIDDLL, "WND_GETVECTOR_BY_NAME");
    wnd_get_composition =
        wanda_helper_functions::loadDLLfunction<int(const char *, const char *, const char *, double *, int *, size_t,
                                                    size_t, size_t)>(hGetProcIDDLL, "WND_GETCOMPOSITION");
    wnd_get_composition_vector =
        wanda_helper_functions::loadDLLfunction<int(const char *, const char *, const char *, double *, int *, size_t,
                                                    size_t, size_t)>(hGetProcIDDLL, "WND_GETCOMPOSITIONVECTOR");
}

wanda_engine *wanda_engine::get_instance(const std::string &wanda_bin)
{
    static wanda_engine _instance(wanda_bin);
    if (_instance._wanda_bin != wanda_bin)
    {
        _instance.~wanda_engine();
        new (&_instance) wanda_engine(wanda_bin);
    }
    return &_instance;
}

wanda_engine::~wanda_engine()
{
    // License auth cleanup
    close_engine();
    FreeLibrary(hGetProcIDDLL);
    hGetProcIDDLL = NULL;
}

void wanda_engine::initialize_engine(const std::string &case_path)
{
    _case_full_path = case_path;
    auto casename = std::make_unique<char[]>(_case_full_path.length() + 1);
    _case_full_path.copy(casename.get(), _case_full_path.length() + 1);

    if (int retval = wnd_main_init(casename.get(), static_cast<int>(_case_full_path.length() + 1)); retval != 0)
    {
        throw std::runtime_error("Error in initializing Wanda engine");
    }
    if (int retval = wnd_load_data(); retval != 0)
    {
        throw std::runtime_error("Error in loading data in Wanda engine");
    }

    if (int retval = wnd_stdy_data_init(); retval != 0)
    {
        throw std::runtime_error("Error in initializing steady calculation in Wanda engine");
    }
    initialized = true;
    time_step = 0;
}

void wanda_engine::run_steady()
{
    if (!initialized)
    {
        throw std::runtime_error("Model not initialized");
    }

    if (int retval = wnd_stdy_comp_init(); retval != 0)
    {
        throw std::runtime_error("Error in initiliazing steady computation in Wanda engine");
    }
    if (int retval = wnd_stdy_compute(); retval != 0)
    {
        throw std::runtime_error("Error in steady computation in Wanda engine");
    }
    steady_computed = true;
    _steady_finished = false;
}

void wanda_engine::run_time_step()
{
    if (!initialized)
    {
        throw std::runtime_error("Model not initiliased");
    }
    if (!steady_computed)
    {
        throw std::runtime_error("Steady not run");
    }
    if (time_step == 0)
    {
        if (int retval = wnd_stdy_final(); retval != 0)
        {
            throw std::runtime_error("Error in finalisation of steady computation in Wanda engine");
        }
        _steady_finished = true;

        if (int retval = wnd_unstdy_init(); retval != 0)
        {
            throw std::runtime_error("Error in finalisation of Wanda engine");
        }
    }
    time_step++;

    if (int retval = wnd_unstdy_compute(&time_step); retval != 0)
    {
        throw std::runtime_error("Error in finalisation of Wanda engine");
    }
}

void wanda_engine::close_engine() noexcept
{
    if (!initialized)
        return;
    if (!_steady_finished)
    {
        int retval = wnd_stdy_final();
        // if (retval != 0)
        // {
        //     throw std::runtime_error("Error in finalisation of case");
        // }
    }
    if (time_step > 0)
    {
        int retval = wnd_unstdy_final();
    }
    int retval = wnd_main_final();
    initialized = false;
    steady_computed = false;
    _steady_finished = false;
    // if (retval != 0)
    // {
    //     throw std::runtime_error("Error in finalisation of case");
    // }
}

double wanda_engine::get_value(std::string comp_name, std::string property)
{
    if (_components.find(comp_name) == _components.end())
    {
        _components.emplace(comp_name, wanda_engine_component(get_comp_handle(comp_name)));
    }
    auto comp = _components.at(comp_name);
    if (comp.properties.find(property) == comp.properties.end())
    {
        comp.properties.emplace(property, get_prop_handle(comp.comp_number, property));
    }
    auto propnum = comp.properties.at(property);
    double value[1];
    int numval = 1;

    if (int retval = wnd_get_values(&comp.comp_number, &propnum, value, &numval); retval != 0)
    {
        if (retval == -1)
            throw std::runtime_error(comp_name + " does not exists");
        if (retval == -2)
            throw std::runtime_error(property + " does not exists");
        throw std::runtime_error("Unknown error");
    }
    return value[0];
}

double wanda_engine::get_value(wanda_component &comp, std::string property)
{
    return get_value(comp.get_complete_name_spec(), property);
}

void wanda_engine::set_value(const std::string &comp_name, const std::string &property, const double value)
{
    if (_components.find(comp_name) == _components.end())
    {
        _components.emplace(comp_name, wanda_engine_component(get_comp_handle(comp_name)));
    }
    auto comp = _components.at(comp_name);
    if (comp.properties.find(property) == comp.properties.end())
    {
        comp.properties.emplace(property, get_prop_handle(comp.comp_number, property));
    }
    auto propnum = comp.properties.at(property);
    int numval = 1;
    double values[1];
    values[0] = value;

    if (int retval = wnd_set_values(&comp.comp_number, &propnum, values, &numval); retval != 0)
    {
        if (retval == -1)
            throw std::runtime_error(comp_name + " does not exists");
        if (retval == -2)
            throw std::runtime_error(property + " does not exists");
        throw std::runtime_error("Unknown error");
    }
}

void wanda_engine::set_value(wanda_component &comp, const std::string &property, double value)
{
    set_value(comp.get_complete_name_spec(), property, value);
}

std::vector<double> wanda_engine::get_vector(std::string comp_name, std::string property)
{
    if (_components.find(comp_name) == _components.end())
    {
        _components.emplace(comp_name, wanda_engine_component(get_comp_handle(comp_name)));
    }
    auto comp = _components.at(comp_name);
    if (comp.properties.find(property) == comp.properties.end())
    {
        comp.properties.emplace(property, get_prop_handle(comp.comp_number, property));
    }
    auto propnum = comp.properties.at(property);
    std::string elements = "Pipe element count";
    int numval = int(get_value(comp_name, elements) + 1);
    std::vector<double> values(numval);

    if (int retval = wnd_get_vector(&comp.comp_number, &propnum, values.data(), &numval); retval != 0)
    {
        if (retval == -1)
            throw std::runtime_error(comp_name + " does not exist");
        if (retval == -2)
            throw std::runtime_error(property + " does not exist");
        if (retval == -3)
            throw std::runtime_error("Storage size to small for returning the vector");
        throw std::runtime_error("Unknown error");
    }
    return values;
}

std::vector<double> wanda_engine::get_vector(wanda_component &comp, std::string property)
{
    return get_vector(comp.get_complete_name_spec(), property);
}

void wanda_engine::finish_unsteady()
{
    int retval = wnd_unstdy_final();
    if (retval != 0)
    {
        throw std::runtime_error("Error in finalisation of unsteady");
    }
}

int wanda_engine::get_comp_handle(std::string complete_name_spec) const
{
    std::string class_name = complete_name_spec.substr(0, complete_name_spec.find(' '));
    std::string name = complete_name_spec.substr(complete_name_spec.find(' ') + 1, complete_name_spec.length());
    auto comp_class = std::make_unique<char[]>(class_name.length() + 1);
    class_name.copy(comp_class.get(), class_name.length() + 1);
    auto comp_name = std::make_unique<char[]>(name.length() + 1);
    name.copy(comp_name.get(), name.length() + 1);

    int comp_handle = wnd_get_component_handle(comp_class.get(), comp_name.get(), class_name.length(), name.length());
    if (comp_handle == -1)
    {
        throw std::runtime_error(complete_name_spec + " does not exist");
    }
    return comp_handle;
}

int wanda_engine::get_prop_handle(int comp_handle, std::string property) const
{
    auto prop_name = std::make_unique<char[]>(property.length() + 1);
    property.copy(prop_name.get(), property.length() + 1);
    int prop_handle = wnd_get_property_handle(&comp_handle, prop_name.get(), property.length());
    if (prop_handle == -1)
    {
        throw std::runtime_error(property + " does not exist");
    }
    if (prop_handle == -3)
    {
        throw std::runtime_error(comp_handle + " does not exist");
    }
    return prop_handle;
}

double wanda_engine::get_start_time() const
{
    double start_time;
    if (int retval = wnd_get_start_time(&start_time); retval != 0)
        throw std::runtime_error("error in wnd_get_start_time");
    return start_time;
}

double wanda_engine::get_end_time() const
{
    double end_time;
    if (int retval = wnd_get_end_time(&end_time); retval != 0)
        throw std::runtime_error("error in wnd_get_end_time");
    return end_time;
}

double wanda_engine::get_current_time() const
{
    double cur_time;
    if (int retval = wnd_get_current_time(&cur_time); retval != 0)
        throw std::runtime_error("error in wnd_get_current_time");
    return cur_time;
}

double wanda_engine::get_delta_t() const
{
    double delta_t;
    if (int retval = wnd_get_delta_t(&delta_t); retval != 0)
        throw std::runtime_error("error in wnd_get_current_time");
    return delta_t;
}
