#ifndef _WANDA_ENGINE_NATIVE_
#define _WANDA_ENGINE_NATIVE_

#include "deltares_helper_functions.h"
#include "unordered_map"

#include <Windows.h>
#include <functional>
#include <wandamodel.h>

#ifdef WANDAMODEL_EXPORT
// #define WANDAMODEL_API __declspec(dllexport)
#define WANDAMODEL_API 
#else
#define WANDAMODEL_API __declspec(dllimport)
#endif
///@private
struct wanda_engine_component
{
    int comp_number = -999;
    std::unordered_map<std::string, int> properties;
};
//!  main class for the Wanda engine.
/*!
The wanda_engine class can be used to run simulations and to evaluate results
during the simulation and adjust values. This is done in memory with the Wanda
engine. The Wanda engine can only be called for one model at the time.
*/
class WANDAMODEL_API wanda_engine
{
  public:
    wanda_engine(const wanda_engine &) = delete;
    wanda_engine(const wanda_engine &&) = delete;
    wanda_engine &operator=(const wanda_engine &) = delete;
    wanda_engine &operator=(wanda_engine &&other) = delete;
    wanda_engine &operator=(const wanda_engine &&) = delete;

    //! Returns the instance of the wanda_engine object
    static wanda_engine *get_instance(const std::string &wanda_bin);
    //! Closes the Wandaengine object
    ~wanda_engine();
    //! Initializes the Wanda engine with the given wandamodel
    /*!
    \param case_path path to the *.wdi file
    */
    void initialize_engine(const std::string &case_path);
    //! Runs a steady state simulation with the selected wandamodel
    void run_steady();
    //! Simulates one time step with the model
    void run_time_step();
    //! Finalizes the transient simulation. This needs to be called to ensure the
    //! files are closed correctly.
    void finish_unsteady();
    //! Closes the engine by finalizing steady and unsteady ensuring the wanda
    //! case files are close correctly.
    void close_engine() noexcept;
    //! Returns the value at the current time step of the given property of the
    //! given component
    /*!
    \param comp_name name of the component for which the value of the given
    property should be returned
    \param property name of the property for which the current value should be returned.
    */
    double get_value(std::string comp_name, std::string property);

    //! Returns the value at the current time step of the given property of the
    //! given component
    /*!
    \param comp wandacomponent object for which the value of the given property
    should be returned
     \param property name of the property for which the current
    value should be returned.
    */
    double get_value(wanda_component &comp, std::string property);

    //! Sets the value at the current time step of the given property of the given
    //! component
    /*!
    *It should be noted that not all properties can be changed or will have an
    effect when they are changed. *This will depend upon the implementation in the
    WandaEngine. The best way to change properties during *simulations is by
    changing the constant value of a constant output value component connect to
    the hydraulic component.
     \param comp_name name of the component for which the
    value of the given property should be set \param property name of the property
    for which the current value should be set. \param value to set the property to
    */
    void set_value(const std::string &comp_name, const std::string &property, double value);
    //! Sets the value at the current time step of the given property of the given
    //! component
    /*!
    *It should be noted that not all properties can be changed or will have an
    effect when they are changed. *This will depend upon the implementation in the
    WandaEngine. The best way to change properties during *simulations is by
    changing the constant value of a constant output value component connect to
    the hydraulic component.
     \param comp Wanda component object for which the
    value of the given property should be set \param property name of the property
    for which the current value should be set. \param value to set the property to
    */
    void set_value(wanda_component &comp,const std::string &property, double value);

    //! Returns the values at the current time step of the given property of the
    //! given pipe
    /*!
    \param comp_name name of the component for which the value of the given
    property should be returned
     \param property name of the property for which the
    current value should be returned.
    */
    std::vector<double> get_vector(std::string comp_name, std::string property);

    //! Returns the values at the current time step of the given property of the
    //! given pipe
    /*!
    \param comp wandacomponent object of the component for which the value of the
    given property should be returned \param property name of the property for
    which the current value should be returned.
    */
    std::vector<double> get_vector(wanda_component &comp, std::string property);

    // TODO include get composition and get composition vector, do not know if it
    // is usefull?
    // std::vector<std::vector<double>> get_composition(std::string
    // comp, std::string property);

    //! Returns the start time of the simulation
    double get_start_time() const;
    //! Returns the end time of the simulation
    double get_end_time() const;
    //! Returns the current time of the simulation
    double get_current_time() const;
    //! Returns the time step of the simulation
    double get_delta_t() const;
    ///@private
    std::size_t wnd_get_hash() const noexcept
    {
        return _object_hash;
    }

  private:
    [[maybe_unused]] const std::size_t _object_hash = std::hash<std::string>{}(std::string("WandaEngine Object"));

    wanda_engine(const std::string &wanda_bin);

    [[nodiscard]] int get_comp_handle(std::string complete_name_spec) const;
    [[nodiscard]] int get_prop_handle(int comp_handle, std::string property) const;
    bool initialized = false;
    bool steady_computed = false;
    int time_step = 0;
    std::string _case_full_path;
    std::string _wanda_bin;
    bool _steady_finished = false;
    std::unordered_map<std::string, wanda_engine_component> _components;

    HINSTANCE hGetProcIDDLL;
    std::function<int(const char *, size_t)> wnd_main_init;
    std::function<int()> wnd_load_data;
    std::function<int()> wnd_stdy_data_init;
    std::function<int()> wnd_stdy_comp_init;
    std::function<int()> wnd_stdy_compute;
    std::function<int()> wnd_stdy_final;
    std::function<int()> wnd_unstdy_init;
    std::function<int(int *)> wnd_unstdy_compute;
    std::function<int()> wnd_unstdy_final;
    std::function<int()> wnd_main_final;
    std::function<int(double *)> wnd_get_current_time;
    std::function<int(double *)> wnd_get_delta_t;
    std::function<int(double *)> wnd_get_start_time;
    std::function<int(double *)> wnd_get_end_time;
    std::function<int(const char *, const char *, size_t, size_t)> wnd_get_component_handle;
    std::function<int(const int *, const char *, size_t)> wnd_get_property_handle;
    std::function<int(const int *, const int *, double *, int *)> wnd_get_values;
    std::function<int(const int *, const int *, double *, int *)> wnd_set_values;
    std::function<int(const int *, const int *, double *, int *)> wnd_get_vector;

    std::function<int(const char *, const char *, const char *, double *, int *, size_t, size_t, size_t)>
        wnd_get_values_by_name;
    std::function<int(const char *, const char *, const char *, double *, int *, size_t, size_t, size_t)>
        wnd_set_values_by_name;
    std::function<int(const char *, const char *, const char *, double *, int *, size_t, size_t, size_t)>
        wnd_get_vector_by_name;
    std::function<int(const char *, const char *, const char *, double *, int *, size_t, size_t, size_t)>
        wnd_get_composition;
    std::function<int(const char *, const char *, const char *, double *, int *, size_t, size_t, size_t)>
        wnd_get_composition_vector;
};
#endif
