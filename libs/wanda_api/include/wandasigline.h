#ifndef _WANDASIGLINE_
#define _WANDASIGLINE_
#include <vector>
#include <wanda_item.h>

#ifdef WANDAMODEL_EXPORT
// #define WANDAMODEL_API __declspec(dllexport)
#define WANDAMODEL_API 
#else
#define WANDAMODEL_API __declspec(dllimport)
#endif

class wanda_component;

//!  Class which provides access to signal lines.
/*!
The wanda signal line class can be used to read and adjust signal lines in the
model.
*/
class WANDAMODEL_API wanda_sig_line : public wanda_item
{
  private:
    //    wanda_sig_line(); // private constructor, never accessed.
    int _sig_line_key = 0;
    std::vector<int> con_point;
    float x_pos = 0;
    float y_pos = 0;
    std::string _signal_line_type;
    wanda_component *_connected_comp_input;
    wanda_component *_connected_comp_output;
    int _input_chan_num;
    int _output_chan_num;
    const static std::string _object_name;
    const std::size_t _object_hash;

  public:
    ///@private
    wanda_sig_line(int, std::string);
    ///@private
    wanda_sig_line();
    //! returns a vector of integer to which channel point the signal line is connected
    std::vector<int> get_con_points() const;
    ///@private
    void set_con_point(std::vector<int>);
    //! Returns the type of signal line (logical, real, sensor)
    std::string get_signal_line_type() const
    {
        return _signal_line_type;
    }
    ///@private
    void set_signal_line_type(std::string type)
    {
        _signal_line_type = type;
    }
    ///@private
    void connect_input(wanda_component &component, int connection_point);
    ///@private
    void connect_output(wanda_component &component, int connection_point);
    //! returns a pointer to the component to which input the signal line is connected
    wanda_component *get_input_component() const
    {
        return _connected_comp_input;
    }
    //! returns a pointer to the component to which output the signal lien is connected
    wanda_component *get_output_component() const
    {
        return _connected_comp_output;
    }
    //! returns the channel number of the control component to which input the signal line is connected
    int get_input_connection_point() const
    {
        return _input_chan_num;
    }
    //! returns the channel number of the control component to which output th signal line is connected
    int get_output_connection_point() const
    {
        return _output_chan_num;
    }
    ///@private
    // returns hash of the object string for object identification (used in c_wanda_engine)
    virtual std::size_t wnd_get_hash()
    {
        return _object_hash;
    }
};

#endif
