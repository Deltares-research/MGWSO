#ifndef _WANDA_MODEL_
#define _WANDA_MODEL_

#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <nefis_file.h>
#include <wanda_diagram_lines.h>
#include <wandacomponent.h>
#include <wandadef.h>
#include <wandanode.h>
#include <wandaproperty.h>
#include <wandasigline.h>
#include <deltares_helper_functions.h>
#include <diagram_text_box.h>

#ifdef WANDAMODEL_EXPORT
// #define WANDAMODEL_API __declspec(dllexport)
#define WANDAMODEL_API 
#else
#define WANDAMODEL_API __declspec(dllimport)
#endif

///@private
struct wanda_output_data_struct
{
    std::vector<std::vector<float>> time_series_data;
    std::vector<std::vector<float>> maximum_value;
    std::vector<std::vector<float>> minimum_value;
    std::vector<std::vector<float>> maximum_value_time;
    std::vector<std::vector<float>> minimum_value_time;
};

///@private
struct tabcol_meta_record
{
    int index = 0;              // index of table in Nefis array
    std::string table_key_next; // Table or column key
    int size = 0;               // length of table (number of records)
};

///@private
struct tab_nefis_info
{
    std::string group_name;
    std::string el_size;
    std::string el_description;
    std::string el_unit_dim;
    std::string el_table_type_key;
    std::string el_table_key_next;
    std::string el_values;
};

//!  main class for the Wanda model interface.
/*!
The wanda_model class is the main interface for the wanda model. This class
gives the user access to all the components and data included in the model.
*/
class WANDAMODEL_API wanda_model final
{
  private:
    const std::string unref = "Unrefrnc";
    const std::string _object_name = "WandaModel Object";
    const std::size_t _object_hash = std::hash<std::string>{}("WandaModel Object");
    std::string wanda_bin;
    int num_time_steps = 0;
    bool initialized = false;
    bool _modified = false;
    char dis_setting = 'X';
    wanda_def *component_definition;
    nefis_file wanda_input_file;
    nefis_file wanda_output_file;
    int number_physical_components = 0;
    int number_control_components = 0;
    int number_physical_nodes = 0;
    int num_signal_lines = 0;
    bool _model_is_corrupt = false;
    int last_key = 0; // is counter to be used if a new key is generated. This is also stored in de WDI file
    std::array<std::string, 5> standard_nodes = {"Hydraulic node", "Heat node", "Zoutlek node", "Gas node",
                                                 "Multi-slurry node"};
    std::unordered_map<std::string, wanda_property> global_vars;
    std::unordered_map<std::string, wanda_property> mode_and_opt;
    std::unordered_map<std::string, wanda_node> phys_nodes; // relates Node_key to wanda_node object]
    std::unordered_map<std::string, wanda_component>
        ctrl_components; // relates Component_key to Control wanda_component objects
    std::unordered_map<std::string, wanda_component>
        phys_components; // relates Component_key to Physical wanda_component objects
    std::unordered_map<std::string, wanda_sig_line> signal_lines; // relates sigline_key to signal line objects
    std::unordered_map<std::string, wanda_diagram_lines> diagram_lines;
    std::unordered_map<std::string, wanda_component *>
        name2_phys_comp; // quick lookup table that relates component id (Complete_spec_descr) to component objects
    std::unordered_map<std::string, wanda_node *>
        name2_phys_node; // quick lookup table that relates node id (Complete_spec_descr) to node objects
    std::vector<std::string> sig_line_keys;
    std::vector<int> deleted_phys_nodes;
    std::vector<int> deleted_ctrl_components;
    std::vector<int> deleted_phys_components;
    std::vector<int> deleted_signal_lines;
    std::unordered_map<std::string, std::unordered_map<std::string, float>> unit_list;
    std::unordered_map<std::string, std::string> case_units;
    std::string unit_group;
    bool tables_loaded = false;
    bool num_cols_loaded = false;
    bool string_col_loaded = false;
    int index_table = 0;
    int index_num_col = 0;
    int index_string_col = 0;
    std::unordered_map<std::string, tabcol_meta_record> table_metainfo_cache;

    std::unordered_map<std::string, wanda_output_data_struct> output_quantity_cache;
    std::unordered_map<std::string, std::vector<int>> output_index_group_cache;
    std::vector<float> simulation_time_steps;
    std::unordered_map<std::string, diagram_text> diagram_text_boxes;
    bool steady_message_not_loaded = true;   // flag to ensure the messages are loaded only once.
    bool unsteady_message_not_loaded = true; // flag to ensure the messages are loaded only once.
    bool new_case_statusflag = false;
    void reset_modified();
    void read_general_items();
    void read_physical_comp();
    void read_nodes();
    void read_signal_lines();
    void read_control_comp();
    void read_phys_component_input();
    void read_phys_node_input();
    void read_ctrl_component_input();
    void read_table(wanda_table &table);
    void load_table_data(wanda_table_data *tab_data, std::string &group_name, std::string &element_name);
    void load_diagram_text_boxes();
    void save_diagram_text_boxes();
    void reload_component_indices();
    static int get_key_index_array(std::vector<std::string>, std::string);
    static std::vector<int> get_key_indices_array(std::vector<std::string>, std::string);
    std::string to_lower(std::string in) const;
    template <typename T> std::string get_unique_key(std::unordered_map<std::string, T> *keys, char prefix, int &i);
    std::string get_unique_key(std::vector<std::string> keys, char prefix, int &i);
    void get_globvar_data();
    void read_mode_and_options();
    int get_view_mask();
    void save_mode_and_options();
    void save_table(wanda_table &table);
    std::string get_new_table_key(std::string description, wanda_table &table);
    void save_unit_system();
    void save_glob_vars();
    bool save_new_phys_comp();
    void save_new_ctrl_comp(wanda_component &comp);
    bool save_new_node();
    void save_new_sig_line(wanda_sig_line &sig_line);
    void save_phys_comp_input();
    void save_ctrl_comp_input(wanda_component &comp);
    void save_node_input(wanda_node &node);
    void save_sig_line_input(wanda_sig_line &sig_lin);
    void load_table();
    std::vector<std::string> get_table_description(const std::string &table_key);
    wanda_item &connect_sensor(wanda_component &h_comp1, int con_point1, wanda_component &sensor, int con_point2);
    void connect_sensor(wanda_node &node, wanda_component &sensor);
    wanda_node &connect_phys_comps(wanda_component &comp1, int con_point1, wanda_component &comp2, int con_point2);
    wanda_sig_line &connect_ctrl_comps(wanda_component &comp1, int con_point1, wanda_component &comp2, int con_point2);
    wanda_sig_line &connect_ctrl2phys_comps(wanda_component &c_comp1, int con_point1, wanda_component &h_comp2,
                                            int con_point2);
    wanda_sig_line &add_sigline(std::string type, std::vector<float> pos);
    void load_message(std::string group);
    void set_unit_factors();
    bool check_name(std::string name);
    bool check_name(std::string name, std::string key);
    bool glob_var_modified();
    int num_timesteps = 0;
    int num_of_species = 0; // Used for MST domain
    int max_num_of_species = 0;
    int species_stride = 0;
    void upgrade_components();
    void upgrade_wdi();
    std::vector<int> parse_date_time(std::string date_time_string);
    std::string get_file_version(const std::string &executable_name);
    std::tuple<bool,bool> check_wanda_version();
    //wanda_helper_functions::wanda_version_number version_number;
    wanda_helper_functions::wanda_version_number version_number = {"0.0.0"};
    void load_wanda_version();

    void calc_hsc(wanda_component& component);
    
    void load_lines_diagram_information();
    void save_lines_diagram_information();  public:
    //! Constructor for wanda_model
    /*!
    * Create the wanda_model object. The constructor will automatically call
    initialize()
    * for this wanda_model object.
    \param casefile Path to the Wanda case input file (*.wdi).
    \param Wandadir Path to the Wanda installation directory.
    \param upgrade_model flag to automaticly upgrade the model when needed, default is false.
    */
    wanda_model(const std::string &casefile, const std::string &Wandadir, bool upgrade_model = false);
    ~wanda_model();
    //! Initializes a wanda_model object
    /*!
    * initialize() opens the wanda case files and initializes the wanda_model
    object. If a *.wdo file
    * exists, initialize() will open that file to and read the meta-information
    and simulation messages.
    * Simulation output is not loaded, users will have to do this manually with
    the reload_output() method.
    \param wdifile name of the Wanda case input file (*.wdi).
    \param upgrade_model, when set to true the model is upgrade when needed
    */
    void initialize(std::string wdifile, bool upgrade_model);
    //! Close the wanda case files
    /*!
     * Closes the wanda case files.
     */
    void close();
    //! Save model input to wanda input file
    /*!
     * save_model_input() checks if the user has modified any input properties of
     * the wanda_model object and saves these changed properties to disk. This
     * also results in deleted components or nodes being removed from the case
     * files and new components or nodes being added to the case files.
     */
    void save_model_input();
    //! Reloads the input data into memory
    /*!
     * reload_input() reloads all the input data from the wanda case files
     * into memory. This will discard any changes that were made to the
     * wanda_model components or properties and which have not been saved.
     */
    void reload_input();
    //! Reads the output data for a single component in the model and stores this in memory.
    /*!
    \param component the component for which output should be read.
    */
    void read_component_output(wanda_component &component);
    //! Reads the output data for a single property in a component and stores this in memory.
    /*!
    \param prop the property for which output should be read.
    */
    void read_prop_output(wanda_property &prop);
    //! Reads the output data for a single node in the model and stores this in memory.
    /*!
    \param node the component for which output should be read.
    */
    void read_node_output(wanda_node &node);
    //! Reads the output data for the entire wanda_model and stores this in memory.
    /*!
     * reload_output reads all the output data from the model into memory, using
     * read_component_output(), and read_node_output() while iterating over all
     * components and nodes in the model
     */
    void reload_output();
    //! Sets modified status
    /*!
    * Sets the modified status of the wanda_model
    \param changed Modified status of the wanda_model
    */
    void set_modified(bool changed)
    {
        _modified = changed;
    };
    //! Gets modified status
    /*!
     * Gets the modified status of the wanda_model and all
     * its underlying components and objects.
     */
    bool is_modified() const;
    //! Returns a reference of a property from the wanda_model
    /*!
    * get_property() returns a wanda_property object that represents the
    * requested property of the wanda_model. This only includes the general
    properties
    * like e.g. Fluid density, Simulation time etc.
    * This does not include the properties
    * of components or nodes. These properties must be requested from a
    wanda_component
    * or wanda_node object.
    \param name Name of the requested property
    */
    wanda_property &get_property(std::string name);
    //! Returns a reference to a component from the wanda_model
    /*!
    * get_component() returns a reference to a wanda_component object that
    represents the
    * requested component of the wanda_model. This works for both physical
    * and control components
    \param componentname Name of the requested component
    */
    wanda_component &get_component(const std::string &componentname);

    //! Return a reference to a node from the wanda_model
    /*!
    * get_node() returns a reference to a wanda_node object that represents the
    * requested node of the wanda_model. This works for all types
    * of nodes
    \param nodename Name of the requested node
    */
    wanda_node &get_node(const std::string &nodename);
    //! Gets a node from the wanda_model
    /*!
    * get_signal_line() returns a wanda_sig_line object that represents the
    * requested signal line
    \param sig_name Name of the requested signal line
    */
    wanda_sig_line &get_signal_line(const std::string &sig_name);

    //! Returns all name of components in model that have a specified keyword.
    /*!
    * get_components_name_with_keyword() iterates over al components in the
    wanda_model object and
    * checks if they have the specified keyword in their keyword list. The method
    returns a std::vector<std::string>
    * object that contains the names of all the components that include the
    specified keyword. \param keyword the keyword that indicates the desired
    components.
    */
    std::vector<std::string> get_components_name_with_keyword(std::string keyword);
    //! Returns pointers to all components in model that have a specified keyword.
    /*!
    * get_components_name_with_keyword() iterates over al components in the
    wanda_model object and
    * checks if they have the specified keyword in their keyword list. The method
    returns a std::vector<wanda_component*>
    * object that contains pointers to all the components that include the
    specified keyword. \param keyword the keyword that indicates the desired
    components.
    */
    std::vector<wanda_component *> get_components_with_keyword(std::string keyword);

    //! Returns pointers to all nodes in model that have a specified keyword.
    /*!
    * get_nodes_with_keyword() iterates over all nodes in the wanda_model object
    and
    * checks if they have the specified keyword in their keyword list. The method
    returns a std::vector<wanda_node*>
    * object that contains pointers to all the nodes that include the specified
    keyword. \param keyword the keyword that indicates the desired nodes.
    */
    std::vector<wanda_node *> get_nodes_with_keyword(std::string keyword);
    //! Returns pointers to all signal_lines in model that have a specified keyword.
    /*!
    * get_signal_line_with_keyword() iterates over all signal_lines in the wanda_model object
    and
    * checks if they have the specified keyword in their keyword list. The method
    returns a std::vector<wanda_signal*>
    * object that contains pointers to all the signal line that include the specified
    keyword. \param keyword the keyword that indicates the desired signal line.
    */
    std::vector<wanda_sig_line *> get_signal_lines_with_keyword(std::string keyword);

    //! Returns names of all signal lane names in model that have a specified keyword.
    /*!
    * get_signal_line_names_with_keyword() iterates over al the signal lines in the wanda_model object and
    * checks if they have the specified keyword in their keyword list. The method  returns a std::vector<std::string>
    * object that contains the names of all the signal lines that include the specified keyword.

    \param keyword the keyword that indicates the desired signal lines.
    */
    std::vector<std::string> get_signal_line_names_with_keyword(std::string keyword);

    //! Returns names of all nodes in model that have a specified keyword.
    /*!
    * get_node_names_with_keyword() iterates over al the nodes in the wanda_model
    object and
    * checks if they have the specified keyword in their keyword list. The method
    returns a std::vector<std::string>
    * object that contains the names of all the nodes that include the specified
    keyword. \param keyword the keyword that indicates the desired nodes.
    */
    std::vector<std::string> get_node_names_with_keyword(std::string keyword);
    //! Returns the number of time steps
    /*!
     * get_num_time_steps() returns an integer that indicates the number of time
     * steps configured in the wanda_model (simulation time / time step).
     */
    int get_num_time_steps();
    //! Returns pointers to all components in a route.
    /*!
    * get_route() iterates over all nodes in the wanda_model object and
    * checks if they have the specified keyword in their keyword list. The method
    returns a std::vector<wanda_component*>
    * object that contains pointers to all the components that include the
    specified keyword, sorted by the connectivity
    * of the components. The positive flow direction of the components is taken
    into account, where the majority of the
    * components determine the positive flow direction of the route.
    \param keyword the keyword that indicates the desired route.
    */
    std::vector<wanda_component *> get_route(std::string keyword, std::vector<int> &dir);
    //! Returns pointers to all pipes in the wandamodel.
    /*!
     * get_all_pipes() iterates over all components in the wanda_model object and
     * checks if it is a pipe component. The method returns a
     * std::vector<wanda_component*> object that contains pointers to all the
     * pipes in the wanda_model.
     */
    std::vector<wanda_component *> get_all_pipes();

    //! Returns pointers to all components.
    /*!
     * get_all_components() returns a std::vector<wanda_component*>
     * object that contains pointers to all the components in the wanda_model.
     */
    std::vector<wanda_component *> get_all_components();

    //! Returns all component names.
    /*!
     * get_all_components_str() returns a std::vector<std::string>
     * object that contains all the names of the components in the wanda_model.
     */
    std::vector<std::string> get_all_components_str();
    //! Returns pointers to all nodes in the wandamodel.
    /*!
     * get_all_nodes() returns a std::vector<wanda_node*>
     * object that contains pointers to all the nodes in the wanda_model.
     */
    std::vector<wanda_node *> get_all_nodes();
    //! Returns pointers to all signal lines in the wandamodel.
    /*!
     * get_all_signal_lines() returns a std::vector<wanda_sig_line*>
     * object that contains pointers to all the signal lines in the wanda_model.
     */
    std::vector<wanda_sig_line *> get_all_signal_lines();
    //! Returns all node names in the wandamodel.
    /*!
     * get_all_nodes_str() returns a std::vector<std::string>
     * object that contains all the names of the nodes in the wanda_model.
     */
    std::vector<std::string> get_all_nodes_str();
    //! Returns all signalline names in the wandamodel.
    /*!
     * get_all_signal_lines_str() returns a std::vector<std::string>
     * object that contains all the names of the signal lines in the wanda_model.
     */
    std::vector<std::string> get_all_signal_lines_str();
    //! creates a new case.
    /*!
    * new_wanda_case() creates a new Wanda case within the current object.
    * The existing wanda model is closed (changes are not saved) and emtpy Wanda
    input file (*.wdi)
    * is created. This input file is then opened and the wanda_model object is
    updated to represent
    * the empty wanda case file.
    \param casename name of the new Wanda case.
    */
    void new_wanda_case(std::string casename);
    //! Copies the data from the wanda model given as input to the current wanda model
    /*!
    *The Wanda model given as input is copied to the current Wanda model.
    *This is done by copying all the components and reconnecting them.
    *Names of components, nodes and signal lines are adjusted if the name already
    *exists in the curent model, this is done by adding a number to the name.
    \param[in] model is wandamodel to add to the current model
    */
    void copy_all_from_model(wanda_model &model, float offset_x_ = 0.0, float offset_y = 0.0);

    //! Adds a wanda component to the wanda_model
    /*!
    * add_component() adds a component of the specified type to the wanda_model at
    * the specified position. It does not connect the component to nodes or signal
    lines.
    * The type name is the unique name of the wanda component, such as "BoundH
    (reservoir)"
    * The position is a vector of two floats, X and Y coordinates in iGrafix
    notation.
    * [0,0] is the left-top corner. The return value is a reference to the newly
    added component
     \param type Type of the component
     \param pos is the position of the component in Igrafix coordinates
    */
    wanda_component &add_component(std::string type, std::vector<float> pos);
    //! Adds a wanda component to the wanda_model
    /*!
    * add_component() adds a component of the specified type to the wanda_model at
    * the specified position and gives this component the specified name. It does
    not
    * connect the component to nodes or signal lines.
    * The type name is the unique name of the wanda component, such as "BoundH
    (reservoir)"
    * The position is a vector of two floats, X and Y coordinates in iGrafx
    notation.
    * [0.0 ,0.0] is the left-top corner of the diagram.
    \param type Type of the component
    \param pos is the position (xy coordinates) of the new component
    \param name name of the new wanda component
    */
    wanda_component &add_component(std::string type, std::vector<float> pos, std::string name);
    //! Adds a copy of the wanda component to the case at the specified position
    /*!
    * add_component() adds a copy of the specified component to the wanda_model at
    * the specified position. It does not connect the component to nodes or signal
    lines.
    * The position is a vector of two floats, X and Y coordinates in iGrafx
    notation.
    * [0.0 ,0.0] is the left-top corner of the diagram.
    * If the name is already existing it is appended with an number. This number
    if chosen such that an unique name is created. The name is clipped to 128 characters.
     \param comp_org orginal component to copy in the model \param
    position position (xy coordinates) of the new component
    */
    wanda_component &add_component(wanda_component *comp_org, std::vector<float> position);
    //! Adds a new node to the wanda_model
    /*!
    * add_node() adds a node of the specified type to the wanda_model at
    * the specified position. It does not connect the node to any components.
    * The type name is the unique name of the node, such as "Heat node init T"
    * The position is a vector of two floats, X and Y coordinates in iGrafx
    notation.
    * [0.0 ,0.0] is the left-top corner of the diagram.
    * the return value is a reference to the node.
    \param type Type of the component
    \param position position (xy coordinates) of the new component
    */
    wanda_node &add_node(std::string type, std::vector<float> position);
    //! Adds a copy of the specified node to the model
    /*!
    * add_node() adds a copy node of the specified node to the wanda_model at
    * the specified position. It does not connect the node to any components.
    * The position is a vector of two floats, X and Y coordinates in iGrafx
    notation.
    * [0.0 ,0.0] is the left-top corner of the diagram.
    * If the name is already existing it is appended with an _number. This number
    if chosen such that an unique name is created
    * Since the length of the name is maximum 15 characters the name is clipped
    when requried.
    * the return value is a reference to the node.
    \param node_org is pointer to the node to copy
    \param position position (xy coordinates) of the new component
    */
    wanda_node &add_node(wanda_node *node_org, std::vector<float> position);

    wanda_diagram_lines &add_diagram_line(wanda_item *item, std::string from_key, std::string to_key,
                                          std::vector<float> x, std::vector<float> y, int color, int line_thickness);
    //! Deletes a component from the wanda_model
    /*!
    * Deletes a component from the wanda_model. The deletion is
    * not saved to the case files until save_model_input() is called.
    \param component the component to be deleted
    */
    void delete_component(wanda_component &component);
    //! Deletes a node from the wanda_model
    /*!
    * Deletes a node from the wanda_model. The deletion is
    * not saved to the case files until save_model_input() is called.
    \param node the component to be deleted
    */
    void delete_node(wanda_node &node);
    //! Deletes a signal line from the wanda_model
    /*!
    * Deletes a signal line from the wanda_model. The deletion is
    * not saved to the case files until save_model_input() is called.
    \param sig_line the component to be deleted
    */
    void delete_sig_line(wanda_sig_line &sig_line);

    //! Connects two components
    /*!
    * The connect() method connects two components to each other. This
    * method will create a new node or signal line if required. If there is
    already a node
    * connected to the specified connection point, that node will be used to
    * connect the components.
    * The connection points to be used must be specified for each component. For
    physical
    * components, the left connection point is generally connection point 1, the
    one on the right side connection point 2.
    * Note that the control connection point of physical components is not
    included in this!
    * For control components the top connection point is connection point 1 and
    the
    * connection points are numbered downward. The input and output connection
    points are
    * numbered separately, so there is an input connection point 1 and an output
    connection point 1.
    * Therefore, order of the component is important when connnecting control
    components. The
    * first component is the control component that outputs the value, the second
    component
    * is the component that takes the value as input.
    \param comp1 the first component to be connected
    \param con_point1 the connection point of the first component to be
    used \param comp2 the second component to be connected
     \param
    con_point2 the connection point of the second component to be used
    */
    wanda_item &connect(wanda_component &comp1, int con_point1, wanda_component &comp2, int con_point2);

    //! Connects two components
    /*!
    * The connect() method connects two components to each other. This
    * method will create a new node or signal line if required. If there is
    already a node
    * connected to the specified connection point, that node will be used to
    * connect the components.
    * The connection points to be used must be specified for each component. For
    physical
    * components, the left connection point is generally connection point 1, the
    one on the right side connection point 2.
    * Note that the control connection point of physical components is not
    included in this!
    * For control components the top connection point is connection point 1 and
    the
    * connection points are numbered downward. The input and output connection
    points are
    * numbered separately, so there is an input connection point 1 and an output
    connection point 1.
    * Therefore, order of the component is important when connnecting control
    components. The
    * first component is the control component that outputs the value, the second
    component
    * is the component that takes the value as input.
    \param comp1 the first component to be connected
    \param con_point1 the connection point of the first component to be
    used \param comp2 the second component to be connected
     \param
    con_point2 the connection point of the second component to be used
    \param node_name name which will be given to the node which connects the two components.
    */
    wanda_item &connect(wanda_component &comp1, int con_point1, wanda_component &comp2, int con_point2, std::string node_name);
    //! Connects a wanda component to a node
    /*!
    * This method will connect a specified wanda component (physical components
    only, control
    * components cannot connect to a node directly) to a specified node.
    * The connection points to be used must be specified for each component. For
    physical
    * components, the left connection point is generally connection point 1. The
    other
    * connection points are then numbered sequentially in clockwise direction.
    Note that
    * the control connection point of physical components is not included in this!
    \param comp1 the first component to be connected
    \param con_point1 the connection point of the first component to be
    used \param node the node to connect to
    */
    void connect(wanda_component &comp1, int con_point1, wanda_node &node);
    //! Merges the two nodes into one node
    /*!
    *merge_nodes() merges the two given nodes into one node. The second node is
    deleted from the model and the connection *of this node are transfered to the
    first node \param node1 first node to merge, this node is kept in the model.
    \param node2 second node the merge, this node is deleted from the model.
    */
    void merge_nodes(wanda_node &node1, wanda_node &node2);
    //! Disconnects a wanda component from a node
    /*!
    * This method will disconnect a specified wanda component from its attached
    node or signal line .
    * The connection points to disconnect must be specified. For physical
    * components, the left connection point is generally connection point 1. The
    other
    * connection points are then numbered sequentially in clockwise direction.
    Note that
    * the control connection point of physical components is the last to be
    numbered! \param[in] component the handle of the component to be disconnected
    \param[in] connection_point the connection point to be disconnected
    */

    void disconnect(wanda_component &component, int connection_point);
    //! Disconnects a wanda control component from a signal line
    /*!
    * This method will disconnect a specified wanda control component from its
    attached signal line.
    * The signal line will be deleted.
    * The connection points to be used must be specified for each component.
    * The input and output connection points are numbered separately, so there is
    * an input connection point 1 and an output connection point 1.
    \param[in] component The handle of the component to be disconnected
    \param[in] connection_point The connection point to be disconnected
    \param[in] input true when the input is disconnected, false when an output
    channel should be disconnected.
    */
    void disconnect(wanda_component &component, int connection_point, bool input);
    //! Run steady state computation
    /*!
     * Run the steady state computation for this wanda_model. changes to
     * input are saved before running the computation. The simulation output
     * will be loaded into memory after the computation has finished.
     */
    void run_steady();

    //! Run unsteady (transient) state computation
    /*!
     * Run the unsteady (transient) state computation for this wanda_model.
     * changes to input are saved before running the computation. If there are
     * changes or steady has not run, the run_steady() will be called before the
     * unsteady computation is performed. The simulation output will be loaded
     * into memory after the computation has finished.
     */
    void run_unsteady();
    void reset_wdo_pointer();
    void resume_unsteady_until(float simulation_time);
    //! Returns a list of the case units for the wanda case
    std::vector<std::vector<std::string>> get_case_units() const;
    //! returns a list of the possible dimensions for the given unit
    /*!
    \param[in] unit the unit for which the dimensions is requested
    */
    std::vector<std::string> get_possible_dimensions(std::string unit) const;
    //! Returns the current unit dimension of the given unit
    /*
     \param[in] unit_key, key of the unit for which the dimension should be given.
     */
    std::string get_current_dim(std::string unit_key);
    //! set the given unit to the given dimension
    /*!
    *Changing a unit will switch the unit system to User units.
    *The unit system will only be saved when save_model_input() is called.
    \param[in] unit for which the dimensions is changed
    \param[in] dimension is the dimension to change the unit to
    */
    void set_unit(std::string unit, std::string dimension);
    //! switch units of the case to SI units
    void switch_to_unit_SI();
    //! switch units of the case to UK units
    void switch_to_unit_UK();
    //! switch units of the case to US units
    void switch_to_unit_US();
    //! switch units of the case to user specified units as stored in the wanda model file
    void switch_to_unit_user();
    ///@private
    void switch_to_unit_Wanda();
    //! Returns the Wanda bin folder
    std::string get_wandabin() const
    {
        return wanda_bin;
    }
    //! Returns the wanda case plus folder
    std::string get_case_path() const
    {
        return wanda_input_file.get_filename();
    }
    std::vector<float> default_position = {10.0, -10.0};
    ///@private
    std::vector<float> get_globvar_hcs();
    //! Recalculates the hydraulic calculated specs of all components. For example the wave speed of a pipe.
    void re_calculate_hcs();
    //! Loads steady message into the components
    void load_steady_messages();
    //! Loads unsteady message into the components
    void load_unsteady_messages();
    //! returns a vector of the simulations time steps
    std::vector<float> get_time_steps() const;
    //! returns true when the wandamodel contains the property
    /*!
    \param property to check whether it exists
    */
    bool has_property(const std::string &property) const;
    //! returns true when the given component exists in the Wanda model, otherwise it returns false
    bool component_exists(const std::string &comp_name) const;
    //! returns true when the given component exists in the Wanda model, otherwise it returns false
    bool component_exists(const wanda_component &comp) const;
    //! returns true when the given node exists in the Wanda model, otherwise it returns false
    bool node_exists(const std::string &node) const;
    //! returns true when the given node exists in the Wanda model, otherwise it returns false
    bool node_exists(const wanda_node &node) const;
    //! returns true when the given signal line exists in the Wanda model, otherwise it returns false
    bool sig_line_exists(const wanda_sig_line &sig_line) const;
    //! returns true when the given signal line exists in the Wanda model, otherwise it returns false
    bool sig_line_exists(const std::string &sig_line) const;
    //! sets the properties of the model to the values loaded from the property template file
    /*!
    \param data structure created with the load property template function.
    */
    void add_data_from_template(std::unordered_map<std::string, wanda_prop_template> data);
    //! Reads the template file and puts the data in the Wandamodel general items
    /*!
    \param template_file is Wanda template file to read data from and store into
    the general items of the wanda model
    */
    void add_data_from_template_file(std::string template_file);
    ///@private
    std::unordered_map<std::string, wanda_component>::iterator begin()
    {
        return phys_components.begin();
    }
    ///@private
    std::unordered_map<std::string, wanda_component>::const_iterator begin() const
    {
        return phys_components.begin();
    }
    ///@private
    std::unordered_map<std::string, wanda_component>::iterator end()
    {
        return phys_components.end();
    }
    ///@private
    std::unordered_map<std::string, wanda_component>::const_iterator end() const
    {
        return phys_components.end();
    }
    ///@private
    std::unordered_map<std::string, wanda_component>::iterator find(const std::string &k)
    {
        return phys_components.find(k);
    }
    ///@private
    std::unordered_map<std::string, wanda_component>::const_iterator find(const std::string &k) const
    {
        return phys_components.find(k);
    }
    ///@private
    wanda_component &operator[](const std::string &k)
    {
        return phys_components[k];
    }
    ///@private
    wanda_component &operator[](std::string &&k)
    {
        return phys_components[k];
    }
    //! upgrades the model to the current Wanda version.
    /*
     * upgrade_model() will replace all obsolete component with their new component
     */
    void upgrade_model();
    //! Changes the given component into a component of the type given by the class sort key
    /*
    \param[in] name, name of the component to change
    \param[in] type, type to change the component to
    */
    void change_comp_type(const std::string &name, const std::string &type);
    //! Changes the given node into a node of the type given by the class sort key
    /*
     \param[in] name, name of the node to change
    \param[in] type, type to change the node to
     */
    void change_node_type(const std::string &name, const std::string &type);
    //! Checks if all input is set in the model
    //! validate_model_input returns a list with components and properties, which
    //! are not yet set by the user.
    std::unordered_map<std::string, std::vector<std::string>> validate_model_input();
    //! Checks if all connection are good in the model
    //! validate_connectivity() returns a list of components and connect points which still need to be connected
    std::unordered_map<std::string, std::vector<int>> validate_connectivity();
    ///@private
    //! returns hash of the object string for object identification (used in c_wanda_engine)
    std::size_t wnd_get_hash() const
    {
        return _object_hash;
    }
    //! Splits the pipe at the given distance from connection point 1
    //! split_pipe() returns the node which is created at the split location
    wanda_node &split_pipe(wanda_component &pipe, float loc);
    //! Merges two pipes into one pipe.
    /*!
    * This is only done when the following conditions are met:
    * 1. Pipes are directly connected
    * 2. No components are connected to the node which is between them
    * 3. Input properties are fully filled in.
    * The following is done:
    * 1. Length is conserved
    * 2. The diameter is set to the diameter of the weighted average area. As weight the length is used
    * 3. Additional losses of pipe2 are not transfer to pipe 1
    * 4. For the wave speed two options exist, which can be set by the option input
    *     option 1: Keeping the water hammer storage constant
    *     option 2: Keeping the travel time constant.
    \param[in] pipe1 is first pipe to merge
    \param[in] pipe2 is second pipe to merge this pipe is deleted from the model
    \param[in] option Switch to select how the water hammer storage is conserved.
    */
    void merge_pipes(wanda_component &pipe1, wanda_component &pipe2, int option);
    //! Switches model to transient mode
    void switch_to_transient_mode();
    //! Switches model to engineering mode
    void switch_to_engineering_mode();
    //! Switches the control On or Off
    /*!
     * /param[in] value, if true control is switch on if false it is switch off
     */
    void switch_control(bool value)
    {
        get_property("Control").set_scalar(value ? 1.0f : 0.0f);
    }
    //! Switches the cavitation On or Off
    /*!
     * /param[in] value, if true cavitation is switch on if false it is switch off
     */
    void switch_cavitation(bool value)
    {
        get_property("Cavitation").set_scalar(value ? 1.0f : 0.0f);
    }
    //! MST module function. Returns the number of species present in this model.
    int get_number_of_species() const
    {
        return num_of_species;
    }
    ///@private
    std::vector<std::string> split_keyword_list(std::string keywords);
    ///@private
    std::string keywords2_list(std::vector<std::string> keyword_list);
    //! returns a list of available hydraulic components which can be added to the model
    std::vector<std::string> get_possible_phys_comp_type()
    {
        return component_definition->get_possible_phys_comp_type();
    }
    //! returns a list of all available ctrl components which can be added to the model
    std::vector<std::string> get_possible_ctrl_comp_type()
    {
        return component_definition->get_possible_ctrl_comp_type();
    }
    //! returns a list of all available nodes which can be added to the model
    std::vector<std::string> get_possible_node_type()
    {
        return component_definition->get_possible_node_type();
    }
    //! changes the name of the component to the given one
    void change_component_name(wanda_component &comp, std::string new_name);
    //! changes the name of the node to the given one
    void change_node_name(wanda_node &node, std::string new_name);
    //! Returns pointers to all properties of the wandamodel.
    /*!
     * get_all_properties() returns a std::vector<wanda_property*>
     * object that contains pointers to all the properties in the wanda_component.
     */
    std::vector<wanda_property *> get_all_properties();
    std::vector<std::string> get_all_properties_string();
    //! return the maximum size of an given Nefis element
    int get_element_size_wdi(std::string &element);
    int get_element_size_def(std::string &element);
    //! Returns list of all possible keywords from all components and nodes
    std::vector<std::string> get_all_keywords() const;
    std::string get_model_version() const
    {
        return version_number.to_string();
    }
    std::string get_def_version()
    {
        return component_definition->get_wanda_version();
        }
            // returns a vector with al the key of all the diagram lines in the model. a specific line cen be exact via the
    // get_diagram_line method.
    std::vector<std::string> get_all_diagram_lines();
    //! Returns a vector of all text boxes which are in stored in the WDI.
    std::vector<diagram_text> get_all_text_boxes();
    //! Adds a text box with the given properties it returns the key to the text box.
    diagram_text& add_text_box(text_field text, int bck_color, int line_thickness, coordinates coor, float width, float height);
    //! Deletes the text box with the given key
    void delete_text_box(const std::string &key);
    //! Returns a reference to a diagram line from the wanda_model
    /*!
    * get_diagram_line() returns a reference to a diagram_line object that
    represents the
    * requested line of the wanda_model.
    \param diagram_line_key Key of the requested line
    */
    wanda_diagram_lines &get_diagram_line(const std::string &diagram_line_key);
    //! Deletes the diagram lien with the given key
    void delete_diagram_line(const std::string &key);
        
};

WANDAMODEL_API inline wanda_model *create_new_wanda_case(const std::string filename, const std::string wanda_dir)
{
    try
    {
        auto *p = new wanda_model(filename, wanda_dir);
        p->new_wanda_case(filename);
        return p;
    }
    catch (std::exception ex)
    {
#ifdef DEBUG
        std::cerr << ex.what() << std::endl;
#endif
        throw;
    }
}

#endif
