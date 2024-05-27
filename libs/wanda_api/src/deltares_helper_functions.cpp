#include <deltares_helper_functions.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <locale>

#include <wandaproperty.h>

namespace wanda_helper_functions
{
std::vector<std::string> split(const std::string &input, char delimeter)
{
    std::stringstream ss(input);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delimeter))
    {
        elems.push_back(item);
    }
    return elems;
}

//! Loads data from a Wanda template file into memory so it can be used to set in the Wandamodel
/*!
 \param filename is the name and path of the template file to load.
 */

std::unordered_map<std::string, wanda_prop_template> load_template(const std::string &filename)
{

    if (auto exists = std::filesystem::exists(filename); !exists)
    {
        throw std::invalid_argument(filename + " does not exit");
    }

    int line_number = 0;
    std::ifstream infile(filename);
    std::string line;
    bool table = false;
    std::unordered_map<std::string, wanda_prop_template> property_template_data;
    std::string table_description;
    std::vector<std::vector<std::string>> data_table;

    while (std::getline(infile, line))
    {
        ++line_number;
        std::vector<std::string> line_tokens = wanda_helper_functions::split(line, '\t');

        if (line_tokens.empty())
            throw std::runtime_error("Parsing error in PTF file");
        if (line_tokens.size() < 2) // record probably indicates start of a table
        {
            if (line_tokens[0].substr(line_tokens[0].size() - 5) != "TABLE")
            {
                throw std::runtime_error("Parsing error in PTF file,  TABLE keyword not found");
            }
            table_description = split(line_tokens[0], ':')[0];
            table = true;
            while (table == true)
            {
                std::getline(infile, line);
                ++line_number;
                auto table_values = wanda_helper_functions::split(line, '\t');
                if (table_values.empty())
                    throw std::runtime_error("Parsing error in PTF table data");
                if (table_values[0] == "END_TABLE")
                {
                    table = false;
                    break;
                }
                data_table.push_back(table_values);
            }
            wanda_prop_template info{.table = data_table};
            property_template_data.emplace(table_description, info);
            data_table.erase(data_table.begin(), data_table.end());
        }
        else
        {
            wanda_prop_template info{.value = line_tokens[1], .table = {}};
            property_template_data.emplace(line_tokens[0], info);
        }
    }
    return property_template_data;
}

// trim from end (in place)
// https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring
void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

} // namespace wanda_helper_functions
