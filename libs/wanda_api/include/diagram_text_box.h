#ifndef _WANDADIAGTEXTBOX_
#define _WANDADIAGTEXTBOX_

#include <deltares_helper_functions.h>


///@private
/// Struct to store coordinate information
////// based on https://stackoverflow.com/questions/32538704/best-container-for-coordinates
struct coordinates
{
    float x, y;
    coordinates(float a, float b) : x(a), y(b)
    {
    }

    coordinates(std::vector<float> a) : x(a.at(0)), y(a.at(1))
    {
    }
    coordinates() = default;
    void set_coordinates(float a, float b)
    {
        x = a;
        y = b;
    }
    void set_coordinates(std::vector<float> a)
    {
        if (a.size() < 2)
        {
            throw std::invalid_argument("Not enough coordinates provided");
        }
        x = a.at(0);
        y = a.at(1);
    }

    std::vector<float> get_coordinates()
    {
        return {x, y};
    }
    void add(float xAdd, float yAdd)
    {
        x += xAdd;
        y += yAdd;
    }

    coordinates operator+=(const coordinates &coordinate)
    {
        add(coordinate.x, coordinate.y);
        return *this;
    }

    coordinates operator+(const coordinates& coordinate)
    {
        return *this += coordinate;
    }
};

//enumerate to store location of the text box
enum text_position
{
    top_right,
    center_right,
    bottom_right,
    top_center,
    center_center,
    bottom_center,
    top_left,
    center_left,
    bottom_left
};


//simple struct to convert the textposition from enum to string back and forth.
//If a non exisiting value is given top left is returned.
struct text_pos_conversion
{
    constexpr static std::array<std::pair<std::string_view, text_position>, 9> data = {
        {{"tl", text_position::top_left},
         {"cl", text_position::center_left},
         {"bl", text_position::bottom_left},
         {"tc", text_position::top_center},
         {"cc", text_position::center_center},
         {"bc", text_position::bottom_center},
         {"tr", text_position::top_right},
         {"cr", text_position::center_right},
         {"br", text_position::bottom_right}}};

    //returns the texposition, if it the input is not avaialbel top left is returned.
    static constexpr text_position sv_to_textpos(std::string_view str)
    {
        auto pos = std::find_if(data.begin(), data.end(), [str](const auto &p) { return p.first == str; });
        if (pos != data.end())
            return pos->second;
        return text_position::top_left;
    }

    static constexpr std::string_view textpos_to_sv(text_position tp)
    {
        auto pos = std::find_if(data.begin(), data.end(), [tp](const auto &p) { return p.second == tp; });
        if (pos != data.end())
            return pos->first;
        return "tl";
    }
};
///@private
/// Struct to store text information for a text box
struct text_field
{
    int color;              // color of the text 
    std::string font;       // font of the text
    int font_size;          // size of the text
    std::string text;      // the actual text
    text_position position; // position in the text box
    text_field(int _color, int _font_size, const std::string& _font, const std::string& _text_position, const std::string& _text)
        : color(_color), font(_font), font_size(_font_size), text(_text)
    {
        position = text_pos_conversion::sv_to_textpos(_text_position);
    }
    text_field() = default;

    std::string_view get_text_position() const
    {
        return text_pos_conversion::textpos_to_sv(position);
    }
};

///@private
/// Struct to store information on text boxes in the diagram
struct diagram_text
{
    std::string key;
    text_field text;
    int background_color; // fil color of the text box
    int line_thickness;   // TODO need to check if this is used
    coordinates top_left;
    float height;
    float width;
    diagram_text(std::string _key, text_field _text, int bck_color, int _line_thickness, coordinates coor, float _width, float _height)
        : key(_key), text(_text), background_color(bck_color), line_thickness(_line_thickness), top_left(coor), height(_height),
          width(_width)
    {}
    diagram_text() = default;
};

#endif
