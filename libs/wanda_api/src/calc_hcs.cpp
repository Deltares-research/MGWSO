#include "calc_hcs.h"

#include "deltares_helper_functions.h"

#include <stdexcept>


wanda_component_dll * wanda_component_dll::get_instance(const std::string &wanda_bin)
{
    static wanda_component_dll _instance(wanda_bin);
    if (_instance._wanda_bin != wanda_bin)
    {
        _instance.~wanda_component_dll();
        new (&_instance) wanda_component_dll(wanda_bin);
    }
    return &_instance;
}

wanda_component_dll::~wanda_component_dll()
{
    FreeLibrary(hGetProcIDDLL);
    hGetProcIDDLL = NULL;
}

wanda_component_dll::wanda_component_dll(const std::string &wanda_bin) : _wanda_bin(wanda_bin)
{
    SetDllDirectoryA(_wanda_bin.c_str());
    hGetProcIDDLL = LoadLibrary("Component64.dll");
    if (!hGetProcIDDLL)
    {
        throw std::runtime_error("Could not load Component64.dll or it's dependencies.");
    }
    calc_hsc = wanda_helper_functions::loadDLLfunction<int(
        const char *, const char *, const int *, float *, float **, float **, const int *, const int *,
        const float *, const int *, float *, const int *, float *, size_t, size_t)>(hGetProcIDDLL, "calc_hcs_c");
    get_error_message_dll = wanda_helper_functions::loadDLLfunction<void(char *, size_t)>(hGetProcIDDLL, "ERRORMSG_HCS");
}

void wanda_component_dll::calc_hydraulic_spec_component(wanda_component &component, std::vector<float> globvars)
{
    int const number_hsc = component.get_number_hsc();
    if (number_hsc == 0)
        return;

    // setting the geometry table all length to the largest value
    // this needs to be doen here, since we do not know if user changed the profile tabel
    if (component.is_pipe() && component.contains_property("Profile"))
    {
        component.get_property("Profile").get_table().resize_table_to_max_column_size();
    }

    std::vector<wanda_property *> hcs;

    std::vector<float> his = component.get_his_values();
    std::vector<float> ht = component.get_height_nodes();

    // TT2 contains pointers to the second column of a table. TT1
    // contains pointers to the rest.
    //TL contains the length of all the tables
    table_data data = component.get_table_data();

    
    char *classname_pt = new char[8];
    std::string const classname = component.get_class_sort_key();
    for (size_t i = 0; i < classname.size(); i++)
    {
        classname_pt[i] = classname[i];
    }
    char mode = 'E';
    if (globvars.back() == float(1.0))
    {
        mode = 'T';
    }
    
    std::vector<float> hcs_result(number_hsc);
    int ni = int(his.size());
    int nglob = int(globvars.size());
    int nht = int(ht.size());
    // calculating the calculated specs
    int retval = calc_hsc(&mode, classname_pt, &ni, his.data(), data.column1.data(), data.column2.data(), data.size_tables.data(), &nglob,
                          globvars.data(), &number_hsc, hcs_result.data(), &nht, ht.data(), 1,
                          classname.size());
    if (retval != 0)
    {
        //auto error = get_error_message();
        component.set_hcs_error(get_error_message());
        //throw std::runtime_error("Error during calc_hsc(): " + error);
    }
    component.set_hsc_results(hcs_result);
    component.set_length_from_hsc(his);
}

std::string wanda_component_dll::get_error_message() const
{
    char error[150]; //length is hard coded in component dll.
    get_error_message_dll(error, 150);
    std::string error_string(error);
    wanda_helper_functions::rtrim(error_string);
    return error_string;
}
