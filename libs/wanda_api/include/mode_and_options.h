#ifndef MODE_AND_OPTIONS
#define MODE_AND_OPTIONS

#include <array>
#include <string>
#include <unordered_map>
#include <wandaproperty.h>

struct mode_and_opt_item
{
    std::string name;
    int bitmask_exponent;
    int default_value;
};

class mode_and_options_base_data
{
  public:
    [[nodiscard]] auto get_mode_and_option_list() const noexcept
    {
        return m_a_opt_it_;
    }
    [[nodiscard]] auto get_default_list() const
    {
        std::unordered_map<std::string, wanda_property> list;
        for (auto &item : m_a_opt_it_)
        {
            list.emplace(std::make_pair(item.name, wanda_property(item.bitmask_exponent, item.name, ' ', ' ', "MAO", "",
                                                                  wanda_property_types::MAO, "")));
            list[item.name].set_group_index(item.bitmask_exponent);
            list[item.name].settype();
            list[item.name].set_scalar(static_cast<float>(item.default_value));
        }
        return list;
    }

  private:
    std::array<mode_and_opt_item, 18> m_a_opt_it_ = {{// mode&option name, Bitmask exponent,  default value
                                                      {"Engineering mode", 0, 1},
                                                      {"System characteristic", 1, 0},
                                                      {"Transient mode", 2, 0},
                                                      {"Cavitation", 3, 0},
                                                      {"Control", 4, 0}, // 5-7 not used
                                                      {"Pump energy", 8, 0},
                                                      {"Distance axis", 9, 1}, // 10-14 not used
                                                      {"Sequence number", 15, 1},
                                                      {"Model name", 16, 1},
                                                      {"Material name", 17, 1},
                                                      {"Reference id", 18, 1}, // 19-20 not used
                                                      {"Name prefix in Chart legends", 21, 1},
                                                      {"Upper limits in pipe", 22, 0},
                                                      {"Lower limits in pipe", 23, 0},
                                                      {"Node name in diagram when editing", 24, 1},
                                                      {"Component name in diagram when editing", 25, 1},
                                                      {"Connect point output in pipe", 26, 1},
                                                      {"Show flow direction in pipe symbol", 27, 0}}};
};

#endif
