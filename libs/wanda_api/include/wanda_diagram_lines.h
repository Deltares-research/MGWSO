#ifndef _WANDADIAGRAMLINES_
#define _WANDADIAGRAMLINES_
#include <string>
#include <vector>
#include <stdexcept>


class wanda_item;

class wanda_diagram_lines
{
  private:
      std::string _key;
      wanda_item* _node_key;
      std::string _from_key;
      std::string _to_key;
      std::vector<float> _x_value;
      std::vector<float> _y_value;
      int _color;
      int _line_thickness;
  public:
      wanda_diagram_lines()
    {
        throw std::runtime_error("Invalid call to default constructor of wanda diagram line");
    }
    wanda_diagram_lines(std::string_view key, wanda_item *item_pointer, std::string_view from_key, std::string_view to_key,
                        int n_coord, std::vector<float> x, std::vector<float> y, int color, int line_thickness)
          : 
          _key(key), _node_key(item_pointer), _from_key(from_key), _to_key(to_key), _x_value(x), _y_value(y),
          _color(color), _line_thickness(line_thickness)
    {
          initialize_coordinates(n_coord, x, y);
    }

     void initialize_coordinates(int n_coord, std::vector<float> x, std::vector<float> y)
      {
          _x_value.resize(n_coord);
          std::copy(x.begin(), x.begin() + n_coord, _x_value.begin());
          _y_value.resize(n_coord);
          std::copy(y.begin(), y.begin() + n_coord, _y_value.begin());
      }


    std::string get_key_as_string() const
    { 
        return _key;
    }

    wanda_item* get_node_key() const
    {
        return _node_key;
    }
    void set_node_key(wanda_item* node_key)
    {
        _node_key = std::move(node_key);
    }


    std::string get_from_key() const
    {
        return _from_key;
    }
    void set_from_key(std::string from_key)
    {
        _from_key =std::move(from_key);
    }

    std::string get_to_key() const
    {
           return _to_key;
    }
    void set_to_key(std::string to_key)
    {
        _to_key = std::move(to_key);
    }

    std::vector<float> get_x_value() const
    {
        return _x_value;
    }
    void set_x_value(std::vector<float> x_value)
    {
        _x_value = std::move(x_value);
    }

    std::vector<float> get_y_value() const
    {
            return _y_value;
    }
    void set_y_value(std::vector<float> y_value)
    {
        _y_value = std::move(y_value);
    }

    int get_color() const
    {
        return _color;
    }
    void set_color(int color)
    {
      _color = color;
    }

    int get_line_thickness() const
    {
        return _line_thickness;
    }

    void set_line_thickness(int line_thickness)
    {
        _line_thickness = line_thickness;
    }
};

#endif