#ifndef _WANDATABLE_
#define _WANDATABLE_
#include <set>
#include <unordered_map>
#include <vector>
#include <string>

#ifdef WANDAMODEL_EXPORT
#define WANDAMODEL_API __declspec(dllexport)
#else
#define WANDAMODEL_API __declspec(dllimport)
#endif

///@private
struct wanda_table_data
{
  public:
    std::vector<std::string> stringtable;
    std::vector<float> floattable;
    std::string _unit;
    std::string _key;
    char _table_type = '-';
    int _index = 0;
    int _col_num = 0;
    char _spec_code = '-';
    std::vector<std::string> _related_description;

    [[nodiscard]] std::size_t get_size() const
    {
        if (_table_type == 'S')
            return stringtable.size();
        else
            return floattable.size();
    }
    void resize(const std::size_t size)
    {
        if (_table_type == 'S')
            stringtable.resize(size);
        else
            floattable.resize(size);
    }
};

//!  Class which provides access to table of properties of components.
/*!
The wanda table class can be used to read and adjust tables of components.
*/
class WANDAMODEL_API wanda_table
{
  private:
    static std::string _object_name;
    std::size_t _object_hash;
    std::vector<std::string> _description;
    bool _is_modified = false;
    std::unordered_map<std::string, wanda_table_data> table_data;

  public:
    wanda_table();
    //! Clears all data from the table
    void clear_table();
    //! Returns all float data which is stored in the table
    std::vector<std::vector<float>> get_float_data();
    //! Sets the float data to the given values
    /*!
     \param values to set the float data to. The order is the same as given by
     get_descriptions()
     */
    void set_float_data(std::vector<std::vector<float>> values);
    //! Returns all string data which is stored in the table
    std::vector<std::vector<std::string>> get_string_data();
    //! Sets the string data to the given values
    /*!
    \param values to set the string data to. The order is the same as given by
    get_descriptions()
    */
    void set_string_data(std::vector<std::vector<std::string>> values);
    //! Returns a list of descriptions of the different columns in the table
    std::vector<std::string> get_descriptions() const;
    //! Returns a list of types (float or string) of the different columns in the
    //! table
    std::vector<char> get_table_types();
    ///@private
    std::string get_key(std::string description);
    ///@private
    void set_key(std::string description, std::string key);
    ///@private
    void set_keys(std::string key);
    ///@private
    char get_spec_code(std::string property);
    ///@private
    void add_column(std::string description, std::string unit, std::string key, char type, int index, int col_num,
                    std::string rel_description, char spec_code);
    //! Sets the given float column to the given float data.
    /*!
      \param description of the column to change
      \param values to change the column to
     */
    void set_float_column(std::string description, std::vector<float> values);
    //! Sets the given string column to the given string data
    /*!
    \param description of the column to change
    \param values to change the column to
    */
    void set_string_column(std::string description, std::vector<std::string> values);
    //! returns the data in the given column
    /*!
     \param description of column to return the input for
     */
    std::vector<float> get_float_column(std::string description);
    //! returns pointer to the data in the given column
    /*!
    \param description of column to return the input for
    */
    float *get_float_column_pnt(std::string description);
    // returns the data in the given column
    /*!
    \param description of column to return the input for
    */
    std::vector<std::string> get_string_column(std::string description);
    ///@private
    int get_index(std::string description);
    //! returns the type of the requested column
    /*!
     \param description of the column for which to return the type
     */
    char get_table_type(std::string description);
    ///@private
    int get_col_num(std::string description);
    //! Returns true if any column in the table has data otherwise it returns false
    bool has_data();
    //! Returns true if the given column has data otherwise it returns false
    /*!
     \param description of column
     */
    bool has_data(std::string description);
    ///@private
    std::vector<std::string> get_related_prop(std::string description);
    ///@private
    bool is_modified() const
    {
        return _is_modified;
    }
    ///@private
    void set_modified(bool modified)
    {
        _is_modified = modified;
    }
    //! Returns the unit of the given column
    /*!
    \param description of column
    */
    std::string get_unit(std::string description);
    //! returns if the table has the given description
    /*!
     \param description of the column to check
     */
    bool has_description(std::string description);
    //! return true when the given column is a string column
    /*!
    \param description of the column to check
    */
    bool is_string_column(std::string description);
    ///@private
    wanda_table_data *get_table_data(std::string description);
    //! Checks whether a table has been correctly filled
    bool check_table();
    //! Resizes the different columns to a specified length. size=0 means all columns are resized to the longest length
    void resize_columns(std::size_t size);
    //! returns the length of the longest column
    std::size_t get_max_column_size() const;
    void resize_table_to_max_column_size();
    //! returns hash of the object string for object identification (used in c_wanda_engine)
    std::size_t wnd_get_hash()
    {
        return _object_hash;
    }
    //! copies data from input table to this table
    void copy_table(wanda_table table_org);
};

#endif
