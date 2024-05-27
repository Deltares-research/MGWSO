#ifndef GLOBVAR
#define GLOBVAR
#include <string>
#include <unordered_map>
#include <vector>

struct globvar_item
{
    std::string name;
    std::string element_name;
    std::string groupname;
    std::string type;
    std::string unit_element;
    std::string unit_description;
    int elsize;
    int elnum;
};

class globvar_base_data
{
  public:
    [[nodiscard]] auto get_data_as_list() const noexcept
    {
        return globvar_data;
    }
    [[nodiscard]] auto get_temp_dep_fluid_descr() const noexcept
    {
        return temp_dep_fluid_descr;
    }
    [[nodiscard]] auto get_choice_lists() const noexcept
    {
        return choice_lists;
    }

  private:
    const std::vector<globvar_item> globvar_data = {
        // Wanda name    name in wdi   Nefis group   type    ELement name unit    number of digits
        {"Gravitational acceleration", "Gravitat_accel", "CALC_CONTR_DATA", "REAL", "UK_Gravitat_acce", "acceleration",
         4, 4},
        {"Atmospheric pressure", "Atmosph_press", "CALC_CONTR_DATA", "REAL", "UK_Atmosph_press", "pressure_abs", 4, 4},
        {"Ambient temperature", "T_ambient", "CALC_CONTR_DATA", "REAL", "UK_T_ambient", "temperature", 4, 4},
        {"Time step", "Time_step", "CALC_CONTR_DATA", "REAL", "UK_Time_step", "time", 4, 4},
        {"Simulation time", "End_time", "CALC_CONTR_DATA", "REAL", "UK_End_time", "time", 4, 4},
        {"Start time", "Start_time", "CALC_CONTR_DATA", "REAL", "UK_Start_time", "time", 4, 1},
        {"Output increment", "Output_increment", "CALC_CONTR_DATA", "INTEGER", "", "", 1, 1},
        {"Refresh interval", "Refresh_interval", "CALC_CONTR_DATA", "INTEGER", "", "time", 1, 1},
        {"Chart time span", "Chart_time_span", "CALC_CONTR_DATA", "REAL", "", "time", 1, 1},
        {"Accuracy H rel", "Accuracy_H_rel", "CALC_CONTR_DATA", "REAL", "UK_Acc_H_rel", "dimless", 4, 4},
        {"Accuracy Q rel", "Accuracy_Q_rel", "CALC_CONTR_DATA", "REAL", "UK_Acc_Q_rel", "dimless", 4, 4},
        {"Accuracy H abs", "Accuracy_H_abs", "CALC_CONTR_DATA", "REAL", "UK_Acc_H_abs", "height", 4, 4},
        {"Accuracy Q abs", "Accuracy_Q_abs", "CALC_CONTR_DATA", "REAL", "UK_Acc_Q_abs", "discharge", 4, 4},
        //{"Cavitation mode",             "Cavitation_mode",  "CALC_CONTR_DATA",  "REAL",     "UK_Acc_Q_abs",     "",
        // 1},
        //{"Control mode",                "Control_mode",     "CALC_CONTR_DATA",  "INTEGER",  "",                 "",
        // 1},
        //{"Message threshold",           "Message_threshld", "CALC_CONTR_DATA",  "INTEGER",  "",                 "",
        // 4},
        {"Test level", "Test_level", "CALC_CONTR_DATA", "INTEGER", "", "", 4, 4},
        {"N itermax", "N_itermax", "CALC_CONTR_DATA", "INTEGER", "", "", 1, 1},
        {"Rheology type", "Fluid_choice", "CALC_CONTR_DATA", "CHOICE", "", "", 1, 1},
        {"Salt conversion", "Salt_choice", "CALC_CONTR_DATA", "CHOICE", "", "", 1, 1},
        {"Density", "Fluid_density", "CALC_CONTR_DATA", "REAL", "UK_Fluid_density", "density", 4, 4},
        {"Bulk modulus", "Bulk_modulus", "CALC_CONTR_DATA", "REAL", "UK_Bulk_modulus", "stress", 4, 4},
        {"Vapour pressure", "Vapour_pressure", "CALC_CONTR_DATA", "REAL", "UK_Vapour_pressu", "pressure_abs", 4, 4},
        {"Kinematic viscosity", "Fluid_viscosity", "CALC_CONTR_DATA", "REAL", "UK_Fluid_viscosi", "viscosity", 4, 4},
        {"Visc. Coeff K in H-B model", "Visc_coeff_K", "CALC_CONTR_DATA", "REAL", "UK_Visc_coeff_K", "dimless", 4, 4},
        {"Exponent n in H-B model", "HB_exp_n", "CALC_CONTR_DATA", "REAL", "UK_HB_exp_n", "dimless", 4, 4},
        {"Yield stress (tau_y)", "Yield_stress", "CALC_CONTR_DATA", "REAL", "UK_Yield_stress", "stress", 4, 4},
        {"Init temp zero flow", "Inittempzeroflow", "CALC_CONTR_DATA", "REAL", "UK_tempzeroflow", "temperature", 4, 4},
        {"Specific Gas constant", "Gas_constant", "CALC_CONTR_DATA", "REAL", "UK_Gas_constant", "specifc_heat", 4, 4},
        {"Gas compressibility", "Gas_compressibil", "CALC_CONTR_DATA", "REAL", "UK_Gas_compressi", "dimless", 4, 4},
        {"Gas ratio sp heat (Cp/Cv)", "Gas_ratio_sp_hea", "CALC_CONTR_DATA", "REAL", "UK_Gas_ratio_sp_", "dimless", 4,
         4},
        {"Convergence criterion", "Convergence_crit", "CALC_CONTR_DATA2", "REAL", "UK_Convergence", "dimless", 4, 4},
        {"Iteration min", "Iteration_min", "CALC_CONTR_DATA2", "INTEGER", "UK_Iteration_min", "", 4, 4},
        {"Iteration max", "Iteration_max", "CALC_CONTR_DATA2", "INTEGER", "UK_Iteration_max", "", 4, 4},
        {"Parameter 1", "Parameter_1", "CALC_CONTR_DATA2", "REAL", "UK_Parameter_1", "dimless", 4, 4},
        {"Parameter 2", "Parameter_2", "CALC_CONTR_DATA2", "REAL", "UK_Parameter_2", "dimless", 4, 4},
        {"Parameter 3", "Parameter_3", "CALC_CONTR_DATA2", "REAL", "UK_Parameter_3", "dimless", 4, 4},
        {"Debug flag", "Debug_flag", "CALC_CONTR_DATA2", "INTEGER", "UK_Debug_flag", "", 4, 4},
        {"Restart interval", "Restart_interval", "CALC_CONTR_DATA2", "INTEGER", "", "", 4, 4},
        {"Pressure reference time series", "Press_ref_time", "CASE_INFORMATION", "CHOICE", "", "", 1, 1},
        {"Pressure reference maximum", "Press_ref_max", "CASE_INFORMATION", "CHOICE", "", "", 1, 1},
        {"Pressure reference minimum", "Press_ref_min", "CASE_INFORMATION", "CHOICE", "", "", 1, 1},
        {"Temp.dep. properties", "HtCol_Temp", "CALC_CONTR_DATA", "TABLE", "temperature", "temperature", 1, 1},
        {"Temp.dep. properties", "HtCol_Dens", "CALC_CONTR_DATA", "TABLE", "density", "density", 1, 1},
        {"Temp.dep. properties", "HtCol_Visc", "CALC_CONTR_DATA", "TABLE", "viscosity", "viscosity", 1, 1},
        {"Temp.dep. properties", "HtCol_Pvap", "CALC_CONTR_DATA", "TABLE", "pressure_abs", "pressure_abs", 1, 1},
        {"Temp.dep. properties", "HtCol_Lcp", "CALC_CONTR_DATA", "TABLE", "specifc_heat", "specifc_heat", 1, 1},
        {"Temp.dep. properties", "HtCol_Lab", "CALC_CONTR_DATA", "TABLE", "pipe th.cond", "pipe th.cond", 1, 1},
    };
    const std::unordered_map<std::string, std::string> temp_dep_fluid_descr = {
        {"HtCol_Temp", "Temperature"},     {"HtCol_Dens", "Density"},      {"HtCol_Visc", "Kin.viscosity"},
        {"HtCol_Pvap", "Vapour pressure"}, {"HtCol_Lcp", "Specific heat"}, {"HtCol_Lab", "Ther Cond coef."}};
    const std::unordered_map<std::string, std::vector<std::string>> choice_lists = {
        {"Rheology type", {"Newtonian", "Non-newtonian (Herschel-Bulkley)"}},
        {"Salt conversion", {"Eckard", "Unesco 81"}},
        {"Pressure reference time series", {"Centreline", "Sofit", "Invert"}},
        {"Pressure reference maximum", {"Centreline", "Invert"}},
        {"Pressure reference minimum", {"Centreline", "Sofit"}}};
};

#endif
