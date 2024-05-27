#ifndef WANDA_CALC_HCS
#define WANDA_CALC_HCS
#include "wandacomponent.h"
#include <Windows.h>
#include <functional>


class wanda_component_dll
{
  public:
    wanda_component_dll(const wanda_component_dll &) = delete;
    wanda_component_dll(const wanda_component_dll &&) = delete;
    wanda_component_dll &operator=(const wanda_component_dll &) = delete;
    wanda_component_dll &operator=(wanda_component_dll &&other) = delete;

    static wanda_component_dll *get_instance(const std::string &wanda_bin);
    ~wanda_component_dll();
    void calc_hydraulic_spec_component(wanda_component &component, std::vector<float> globvars);    

  private:
    [[maybe_unused]] const std::size_t _object_hash = std::hash<std::string>{}(std::string("WandaComponentDll Object"));

    wanda_component_dll(const std::string &wanda_bin);

    std::string _wanda_bin;
    HINSTANCE hGetProcIDDLL;
    std::function<int(const char *, const char *, const int *, float *, float **, float **,
                      const int *, const int *, const float *, const int *, float *, const int *, float *, size_t,
                      size_t)>
        calc_hsc;
    std::function<void(char *, size_t)> get_error_message_dll;
    std::string get_error_message() const;
};
#endif
