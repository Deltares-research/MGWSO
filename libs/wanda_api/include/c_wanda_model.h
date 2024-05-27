#ifndef C_WANDA_MODEL
#define C_WANDA_MODEL
#include <string>

class wanda_component;

//!
//! @file c_wanda_model.h
//! @brief C interface functions for the wanda_model objects. This allows access to the
//! wanda_model functionality for non-Object oriented languages.
//!
extern "C"
{
    //! Initializes a wanda_model
    /*!
    * Opens the wanda case files and loads a wanda model into memory. If a *.wdo file
    * exists, it will open that file to and read the meta-information and simulation messages.
    * Simulation output is not loaded, users will have to do this manually with the wnd_reload_model_output() function.
    \param wanda_case Path to the Wanda case input file (*.wdi).
    \param wandadir Path to the Wanda installation directory.
    \return Handle of the loaded model, NULL in case of an error
    */
    void *wnd_load_wanda_model(const char *wanda_case, const char *wandadir);

    //! Close the wanda case files
    /*!
    * Closes the wanda case files.
    \param model_handle Handle of the wanda_model
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_close_wanda_model(void *model_handle);

    //! Save model input to wanda input file
    /*!
    * wnd_save_model_input() checks if the user has modified any input properties of the
    * wanda model and saves these changed properties to disk. This also results in
    * deleted components or nodes being removed from the case files and new components or
    * nodes being added to the case files.
    \param model_handle Handle of the wanda_model
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_save_model_input(void *model_handle);

    //! Create new Wanda case
    /*!
    * Closes the existing wanda case files and creates a new empty
    * wanda case with the user specified name. The new wanda case file
    * is then loaded into memory using the specified handle (replacing the
    * existing wanda case object.
    \param model_handle Handle of the wanda_model
    \param new_case_name file name of the new wanda case files
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_new_wanda_case(void *model_handle, const char *new_case_name);

    //! Create new Wanda case
    /*!
    * Creates a new empty wanda case with the user specified name. The new
    * wanda case file is then loaded into memory and a handle to the new
    * wanda case is returned to the caller
    \param new_case_name file name of the new wanda case files
    \param wandadir Path to the Wanda installation directory.
    \return Handle of the new wanda model, NULL in case of an error
    */
    void *wnd_create_new_wanda_case(const char *new_case_name, const char *wandadir);

    //! Reloads the input data into memory
    /*!
    * wnd_reload_model_input() reloads all the input data from the wanda case files
    * into memory. This will discard any changes that were made to the
    * wanda_model components or properties.
    \param model_handle Handle of the wanda_model
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_reload_model_input(void *model_handle);

    //! Reads the output data for the entire wanda_model and stores this in memory.
    /*!
    * wnd_reload_model_output reads all the output data from the model into memory.
    \param model_handle Handle of the wanda_model
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_reload_model_output(void *model_handle);

    //! Returns the number of timesteps in the case.
    /*!
    * wnd_get_num_timesteps returns the number of timesteps in the case.
    \param model_handle Handle of the wanda_model
    \return number of time step in the model, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_num_time_steps(void *model_handle);

    //! Gets the number of components in a model
    /*!
    * wnd_get_num_components() gets the number of physical and control components in a model
    \param model_handle Handle of the wanda_model
    \return number of component in the model, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_num_components(void *model_handle);

    //! Gets the number of nodes in a model
    /*!
    * wnd_get_num_components() gets the number of physical and control components in a model
    \param model_handle Handle of the wanda_model
    \return number of nodes in the model, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_num_nodes(void *model_handle);

    //! Gets the number of pipes in a model
    /*!
    * wnd_get_num_pipes() gets the number of pipes in a model
    \param model_handle Handle of the wanda_model
    \return number of pipes in the model, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_num_pipes(void *model_handle);

    //! Gets the handles of all components in the wanda case
    /*!
    * wnd_get_all_component_handles() retrieves the handles of all components in the wanda
    * case and returns these in a buffer supplied by the user.
    \param[in] model_handle Handle of the model
    \param[out] buffer Pointer to a void* buffer where the handles will be stored
    \param[in] size Size of the buffer in bytes
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_all_component_handles(void *model_handle, void **buffer, int size);

    //! Gets the handles of all nodes in the wanda case
    /*!
    * wnd_get_all_node_handles() retrieves the handles of all nodes in the wanda
    * case and returns these in a buffer supplied by the user.
    \param[in] model_handle Handle of the model
    \param[out] buffer Pointer to a void* buffer where the handles will be stored
    \param[in] size Size of the buffer in bytes
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_all_node_handles(void *model_handle, void **buffer, int size);

    //! Gets the handles of all pipes in the wanda case
    /*!
    * wnd_get_all_pipe_handles() retrieves the handles of all pipes in the wanda
    * case and returns these in a buffer supplied by the user.
    \param[in] model_handle Handle of the model
    \param[out] buffer Pointer to a void* buffer where the handles will be stored
    \param[in] size Size of the buffer in bytes
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_all_pipe_handles(void *model_handle, void **buffer, int size);

    //! Gets a component from the wanda_model
    /*!
    * get_component() returns a wanda_component object that represents the
    * requested component of the wanda_model. This works for both physical
    * and control components
    \param model_handle Handle of the wanda_model
    \param component_name Name of the requested component
    \return Handle of the requested component, NULL in case of an error
    */
    void *wnd_get_component_handle(void *model_handle, const char *component_name);

    //! Gets a node from the wanda_model
    /*!
    * wnd_get_node_handle() returns a handle for a wanda_node object that represents the
    * requested node of the wanda_model. This works for all types of nodes
    \param model_handle Handle of the wanda_model
    \param node_name Name of the requested node
    \return Handle of the requested node, NULL in case of an error
    */
    void *wnd_get_node_handle(void *model_handle, const char *node_name);

    //! Gets handle for a property
    /*!
    * wnd_get_model_property_handle() returns a handle for a wanda model property. This
    * includes the time parameters, accuracy parameters and mode&options parameters.
    \param model_handle Handle of the wanda_model
    \param property_name Name of the requested node
    \return Handle of the requested property, NULL in case of an error
    */
    void *wnd_get_model_property_handle(void *model_handle, const char *property_name);

    //! Gets handle for a property
    /*!
    * wnd_get_item_property_handle() returns a handle for a wanda component,
    * node or signal line object.
    \param item_handle Handle of the component, node or signal line
    \param property_name Name of the requested node
    \return Handle of the requested property, NULL in case of an error
    */
    void *wnd_get_item_property_handle(void *item_handle, const char *property_name);

    //! Gets the value of the property
    /*!
    * wnd_get_property_value() retrieves the scalar numerical value of the property
    \param[in] property_handle Handle of the property
    \param[out] pvalue Pointer to a floating point variable where the value must be stored
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_property_value(void *property_handle, float *pvalue);

    //! Sets the value of the property
    /*!
    * wnd_set_property_value() sets the scalar numerical value of the property
    \param[in] property_handle Handle of the property
    \param[in] value floating point value
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_set_property_value(void *property_handle, float value);

    //! Gets the string value of the property
    /*!
    * wnd_get_property_value_str() retrieves the string value of the property
    \param[in] property_handle Handle of the property
    \param[out] buffer Pointer to a character buffer where the value must be stored
    \param[in] buffersize size of the buffer in bytes
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_property_value_str(void *property_handle, char *buffer, size_t buffersize);
    int wnd_get_size_property_value_str(void *property_handle);
    //! Sets the value of the property
    /*!
    * wnd_set_property_value() sets the string value of the property
    \param[in] property_handle Handle of the property
    \param[in] string pointer to a null-terminated string
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_set_property_value_str(void *property_handle, const char *string);

    //! Adds a component to the model
    /*!
    * add_component() adds a component of the specified type to the wandamodel at
    * the specified position. It does not connect the component to nodes or signal lines.
    * The type name is the unique name of the wanda component, such as "BoundH (reservoir)"
    * The x_pos and y_pos arguments indicate the position of the new component, X and Y
    * coordinates in iGrafx notation. [0.0 ,0.0] is the left-top corner of the diagram.
    \param[in] model_handle Handle of the model
    \param[in] component_type_name type name of the new component
    \param[in] x_pos X position of the new component
    \param[in] y_pos Y position of the new component
    \return Handle of the new component, NULL in case of an error. An error message is written to stderr
    */
    void *wnd_add_component(void *model_handle, const char *component_type_name, float x_pos, float y_pos);

    //! Adds a node to the model
    /*!
    * add_node() adds a node of the specified type to the wanda_model at
    * the specified position. It does not connect the node to any components.
    * The type name is the unique name of the node, such as "Heat node init T"
    * The x_pos and y_pos arguments indicate the position of the new node, X and Y
    * coordinates in iGrafx notation. [0.0 ,0.0] is the left-top corner of the diagram.
    \param[in] model_handle Handle of the model
    \param[in] node_type_name type name of the new node
    \param[in] x_pos X position of the new component
    \param[in] y_pos Y position of the new component
    \return Handle of the new node, NULL in case of an error. An error message is written to stderr
    */
    void *wnd_add_node(void *model_handle, const char *node_type_name, float x_pos, float y_pos);

    //! delete a component from the model
    /*!
    * wnd_delete_component() disconnects a component from the other components it's connected
    * to and deletes the component from the model
    \param[in] model_handle Handle of the property
    \param[in] component_handle handle of the component to be deleted
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_delete_component(void *model_handle, void *component_handle);

    //! delete a node from the model
    /*!
    * wnd_delete_node() disconnects a node from the other components it's connected
    * to and deletes the node from the model
    \param[in] model_handle Handle of the property
    \param[in] node_handle handle of the node to be deleted
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_delete_node(void *model_handle, void *node_handle);

    //! delete a signal line from the model
    /*!
    * wnd_delete_signal_line() disconnects a signal line from the other components it's connected
    * to and deletes the signal line from the model
    \param[in] model_handle Handle of the property
    \param[in] sigline_handle handle of the signal line to be deleted
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_delete_signal_line(void *model_handle, void *sigline_handle);

    //! Connects two components
    /*!
    * The connect() method connects two components to each other. This
    * method will create a new node if required. If there is already a node
    * connected to the specified connectionpoint, that node will be used to
    * connect the components.
    * The connection points to be used must be specified for each component. For physical
    * components, the left connection point is generally connection point 1. The other
    * connection points are then numbered sequentially in clockwise direction. Note that
    * the control connection point of physical components is not included in this!
    * For control components the top connection point is connection point 1 and the
    * connection points are numbered downward. The input and output connection points are
    * numbered separately, so there is an input connection point 1 and an output connection point 1.
    * Therefore, order of the component is important when connnecting control components. The
    * first component is the control component that outputs the value, the second component
    * is the component that takes the value as input.
    \param[in] model_handle Handle of the property
    \param[in] component1 the first component to be connected
    \param[in] connection_point1 the connection point of the first component to be used
    \param[in] component2 the second component to be connected
    \param[in] connection_point2 the connection point of the second component to be used
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    void *wnd_connect_components(void *model_handle, void *component1, int connection_point1, void *component2,
                                 int connection_point2);

    //! Connects a wanda component to a node
    /*!
    * This method will connect a specified wanda component (physical components only, control
    * components cannot connect to a node directly) to a specified node.
    * The connection points to be used must be specified for each component. For physical
    * components, the left connection point is generally connection point 1. The other
    * connection points are then numbered sequentially in clockwise direction. Note that
    * the control connection point of physical components is not included in this!
    \param[in] model_handle Handle of the property
    \param[in] component1 the first component to be connected
    \param[in] connection_point1 the connection point of the first component to be used
    \param[in] node the node to connect to
    */
    int wnd_connect_component_to_node(void *model_handle, void *component1, int connection_point1, void *node);

    //! Disconnects a wanda component from a node
    /*!
    * This method will disconnect a specified wanda component from its attached node or signal line.
    * The connection points to be used must be specified for each component. For physical
    * components, the left connection point is generally connection point 1. The other
    * connection points are then numbered sequentially in clockwise direction. Note that
    * the control connection point of physical components is not included in this!
    * For control components the top connection point is connection point 1 and the
    * connection points are numbered downward. The input and output connection points are
    * numbered separately, so there is an input connection point 1 and an output connection point 1.
    * Therefore, order of the component is important when connnecting control components. The
    * first component is the control component that outputs the value, the second component
    * is the component that takes the value as input.
    \param[in] model_handle Handle of the property
    \param[in] component_handle The handle of the component to be disconnected
    \param[in] connection_point The connection point to be disconnected
    */
    int wnd_disconnect_component(void *model_handle, void *component_handle, int connection_point);

    //! Sets the position property of an item
    /*!
    * wnd_set_item_position() sets the X,Y position property of an item (a Wanda component,
    * node or signal line). This property is used for drawing the hydraulic model in a graphical
    * user interface
    \param[in] item_handle Handle of the property
    \param[in] x_pos Value of the X position
    \param[in] y_pos Value of the Y position
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_set_item_position(void *item_handle, float x_pos, float y_pos);

    //! Gets the position property of an item
    /*!
    * wnd_get_item_position() gets the X,Y position property of an item (a Wanda component,
    * node or signal line). This property is used for drawing the hydraulic model in a graphical
    * user interface
    \param[in] item_handle Handle of the property
    \param[out] x_pos Pointer to a floating point value where the X position value will be stored
    \param[out] y_pos Pointer to a floating point value where the Y position value will be stored
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_item_position(void *item_handle, float *x_pos, float *y_pos);

    //! Gets the complete name specification of an item
    /*!
    * wnd_get_item_name_spec() gets the complete name specification of an item (a Wanda component,
    * node or signal line). This is used for identifying components by name
    \param[in] item_handle Handle of the property
    \param[out] buffer Pointer to a character buffer where the name specification will be stored
    \param[in] buffersize Size of the buffer in bytes
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_item_name_spec(void *item_handle, char *buffer, size_t buffersize);
    //! returns the length of the complete name of the tiem
    int wnd_get_length_item_name_spec(void *item_handle);
    //
    //! Gets the name of an item
    /*!
    * wnd_get_item_name() gets the name of an item (a Wanda component, node or signal line). This is
    * used for identifying components by name. The name of an item does not have to be unique.
    \param[in] item_handle Handle of the property
    \param[out] buffer Pointer to a character buffer where the name will be stored
    \param[in] buffersize Size of the buffer in bytes
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_item_name(void *item_handle, char *buffer, size_t buffersize);

    //! Gets the class name of an item
    /*!
    * wnd_get_item_classname() gets the classname of an item (a Wanda component, node or signal line). This is
    * used for identifying components by class. The classname of an item does not have to be unique.
    \param[in] item_handle Handle of the property
    \param[out] buffer Pointer to a character buffer where the classname  will be stored
    \param[in] buffersize Size of the buffer in bytes
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_item_classname(void *item_handle, char *buffer, size_t buffersize);

    //! Gets the class name of an item
    /*!
    * wnd_get_item_classname() gets the classname of an item (a Wanda component, node or signal line). This is
    * used for identifying components by class. The classname of an item does not have to be unique.
    \param[in] item_handle Handle of the property
    \param[out] buffer Pointer to a character buffer where the classname  will be stored
    \param[in] buffersize Size of the buffer in bytes
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_item_type(void *item_handle);

    //! Gets the number of elements for an item
    /*!
    * wnd_get_num_elements() gets the number of elements for an property. At this time, only pipes have elements.
    * If this function is called with other components, it will return 0.
    \param[in] property handle Handle of the property
    \return number of elements in the component, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_num_elements(void *prop_handle);

    //! Returns the number of connection points of a component
    /*!
    * wnd_get_num_connectpoints returns the number of connection points of a component
    \param component_handle Handle of the wanda component
    \return number of connection points for the component, 0 = success, -1 indicates an error, a message is written to
    stderr
    */
    int wnd_get_num_connectpoints(void *component_handle);

    int wnd_get_extremes_min_pipe(void *prop_handle, float *buffer, const size_t bufsize);
    int wnd_get_extremes_tmin_pipe(void *prop_handle, float *buffer, const size_t bufsize);
    int wnd_get_extremes_max_pipe(void *prop_handle, float *buffer, const size_t bufsize);
    int wnd_get_extremes_tmax_pipe(void *prop_handle, float *buffer, const size_t bufsize);

    int wnd_get_extremes_min(void *prop_handle, float *result);
    int wnd_get_extremes_max(void *prop_handle, float *result);
    int wnd_get_extremes_tmin(void *prop_handle, float *result);
    int wnd_get_extremes_tmax(void *prop_handle, float *result);

    void *wnd_get_connected_node(void *comp_handle, int connectionpoint);
    int wnd_get_connected_components(void *node_handle, void **buffer, const size_t buffersize);
    int wnd_get_num_connected_components(void *node_handle);
    int wnd_contains_property(void *item_handle, char *property_name);

    //! Checks if a property contains a time series or not.
    /*!
    * Checks if a property contains a time series or not
    \param property_handle Handle of the wanda property
    \return Error code: 0 = true, 1=false. Return value = -1 indicates an error, a message is written to stderr
    */
    int wnd_has_series(void *property_handle);

    int wnd_get_series(void *property_handle, float *buffer, const size_t buffersize);
    int wnd_get_time_series_pipe(void *property_handle, int element, float *buffer, const size_t buffersize);
    int wnd_get_series_pipe(void *property_handle, float *buffer, const size_t buffersize);

    //! Gets the IsFlipped status of the component in the Igrafix diagram.
    /*!
    * Gets the IsFlipped status of the component in the Igrafix diagram.
    \param component_handle Handle of the wanda component
    \param status pointer to an integer where the status should be written to, -1 = flipped, 0 = not flipped
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_flipped_status(void *component_handle);

    //! Gets the Disused status of the component.
    /*!
    * Gets the Disused status of the component.
    \param component_handle Handle of the wanda component
    \param status pointer to an integer where the status should be written to, 0 = False, 1 = True
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_get_flipped_status(void *component_handle);

    //! Run steady computation
    /*!
    * Run the steady state computation for this wanda_model. changes to
    * input are saved before running the computation. The simulation output
    * will be loaded into memory after the computation has finished.
    \param model_handle Handle of the wanda_model
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_run_steady(void *model_handle);

    //! Run transient computation
    /*!
    * Run the unsteady (transient) state computation for this wanda_model. changes to
    * input are saved before running the computation. If there are changes, the
    * run_steady() will be called before the unsteady computation is performed.
    * The simulation output will be loaded into memory after the computation has finished.
    \param model_handle Handle of the wanda_model
    \return Error code: 0 = success, -1 indicates an error, a message is written to stderr
    */
    int wnd_run_unsteady(void *model_handle);
    //! Splits the pipe at the given distance from connection point 1
    //! returns the nod which is created at the split location
    void *wnd_split_pipe(void *comp, float loc);

    //! Returns the number of properties which still need to be filled in. Can be used to create a buffer for
    //! validate_model_input
    int wnd_get_number_results_val_model(void *model_handle);
    //! Checks the model and returns a list with components and properties, which
    //! are not yet provided byt the user
    int wnd_validate_model_input(void *model_handle, char **comps, int comps_size, char **props, int props_size);
    //! Returns the number of connection points which are not connected can be sued to call wnd_check_connectivity
    int wnd_get_number_results_check_con_model(void *model_handle);
    //! Returns a list of components and connect points which are not connected yet and should be connected
    int wnd_check_connectivity(void *model_handle, char **comps, const std::size_t comp_size, int *con_points,
                               const std::size_t cpoints_size);
    //! Merges two pipes into one pipe.
    /*!
    * This is only done when the following conditions are met:
    * 1. Pipes are directly connected
    * 2. No components are connected to the node which is between them
    * 3. Input properties are fully filled in.
    * The following is done:
    * 1. Length is conserved
    * 2. The diameter is set the the diameter of the weighted average area. As weight the length is used
    * 3. Additional losses of pipe2 are not transfer to pipe 1
    * 4. For the wave speed two options exist, which can be set by the option input
    *     option 1: Keeping the water hammer storage constant
    *     option 2: Keeping the travel time constant.
    \param[in] pipe1, first pipe to merge
    \param[in] pipe2, second pipe to merge this pipe is deleted from the model
    \param[in] option Switch to select how the water hammer storage is conserved.
    */
    int wnd_merge_pipes(void *model, void *pipe1, void *pipe2, int option);
    // Can be used to determine the number of components in a route
    int wnd_get_size_of_route(void *model, const char *keyword);
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
    int wnd_get_route(void *model, const char *keyword, void **components, int *dir, const size_t size);
    int wnd_get_size_of_phys_comp_type(void *model);
    //! returns a list of available hydraulic components which can be added to the model
    int wnd_get_possible_phys_comp_type(void *model, char **types, const size_t size);
    //! returns a list of all available keywords
    int wnd_get_all_keywords(void *model, char **types, const size_t size);
    int wnd_get_size_of_ctrl_comp_type(void *model);
    int wnd_get_size_all_keywords(void *model);
    //! returns a list of all available ctrl components which can be added to the model
    int wnd_get_possible_ctrl_comp_type(void *model, char **types, const size_t size);
    int wnd_get_size_of_node_type(void *model);
    //! returns a list of all available nodes which can be added to the model
    int wnd_get_possible_node_type(void *model, char **types, const size_t size);
    //! switches the model to transient mode
    int wnd_switch_to_transient_mode(void *model);
    //! switches the model to engineering mode
    int wnd_switch_to_engineering_mode(void *model);
    //! changes the given component to the given type
    /*!
    \param component, name of to the component to change
    \param type, type name of the component to which is should be changed.
    */
    int wnd_change_component_type(void *model, const char *component, const char *type);
    //! changes the given component to the given type
    /*!
    \param node, name of to the component to change
    \param type, type name of the component to which is should be changed.
    */
    int wnd_change_node_type(void *model, const char *node, const char *type);
    //! switches the model to SI units
    int wnd_switch_to_SI_unit(void *model);
    //! switches the model to user units
    int wnd_switch_to_user_unit(void *model);
    //! Returns a pointer to the signal line with the given name
    void *wnd_get_signal_line(void *model, const char *name);
    //! Returns 1 when the item has the given keyword 0 when not and -1 means error
    int wnd_has_keyword(void *item, const char *keyword);
    //! adds the given keyword to the item
    int wnd_add_keyword(void *item_pointer, const char *keyword);
    //! removes the given keyword from the item
    int wnd_remove_keyword(void *item_pointer, const char *keyword);
    //! Set the action table to either true (1) or false (0)
    int wnd_set_action_table(void *item, const int status);
    //! returns whether the action table is used (1) or not (0)
    int wnd_is_action_table_used(void *item);
    //! returns the signal lines connected to the given connect point of the component
    int wnd_get_connected_signal_lines(void *item, const int con_point, const int input, void **buffer,
                                       const size_t size);
    //! returns the number of connected signal lines
    int wnd_get_number_of_connected_signal_lines(void *item, int con_point, int input);

    //! returns 1 when the component is a pipe otherwise 0
    int wnd_is_pipe(void *comp_handle);
    //! Returns the input component of a signal line
    void *wnd_get_input_component(void *signal_line_handle);
    //! Returns the output component of a signal line
    void *wnd_get_output_component(void *signal_line_handle);
    //! returns the unit factor for the given property
    int wnd_get_unit_factor(void *prop_handle, float *factor);

    //! returns the size of the column.
    int wnd_get_table_size(void *table, const char *description);
    //! returns the values of the given float column
    int wnd_get_float_column(void *table, const char *description, float *values, const size_t size);
    //! sets the values of the given float column
    int wnd_set_float_column(void *table_handle, const char *description, float *values, const size_t size);
    //! loads data from the template file into the given wanda model.
    int wnd_load_data_from_template_to_model(void *model_pointer, const char *template_file);
    void *wnd_get_property_table(void *prop_handle);
    //! returns all handles of components which have the given keyword
    int wnd_get_components_with_keyword(void *model_handle, const char *keyword, void **handles, const size_t size);
    //! returns number of components with the given keyword, usefull to size the buffer
    int wnd_get_number_of_components_with_keyword(void *model_handle, const char *keyword);
    //! returns a float array of all the time steps of the Wanda case
    int wnd_get_time_steps(void *model_handle, float *time_steps, const size_t size);
    //! returns the number of properties in the model
    int wnd_get_number_of_model_properties(void *model_handle);
    //! returns list of pointers to the properties of the wandamodel
    int wnd_get_model_properties(void *model_handle, void **properties, const size_t size);
    //! returns list of strings to the properties of the wandamodel
    int wnd_get_model_properties_string(void *model_handle, char *properties, const size_t size);
    int wnd_get_max_string_size_properties_model(void *model_handle);
    //! returns a list of properties in the given item
    int wnd_get_all_item_properties(void *item_handle, void **properties, const size_t size);
    int wnd_get_all_item_properties_string(void *model_handle, void *item_handle, char *properties, int size);
    int wnd_get_number_of_item_properties(void *item_handle);
    int wnd_get_size_item_property_string(void *model_handle);
    //! returns the name of the property
    int wnd_get_property_description(void *property_handle, char *description, int size);
    int wnd_get_size_property_description(void *property_handle);
    //! return the descriptions which are in the table
    int wnd_get_table_descriptions(void *model_handle, void *table_handle, char *descriptions, const size_t size);
    int wnd_get_number_table_descriptions(void *table_handle);
    int wnd_get_string_size_table_description(void *model_handle);
    //! return 1 when the component exists in the Wanda model otherwise 0
    int wnd_component_exists(void *model, char *name);
    //! Return 1 when the node exists in the Wanda model otherwise 0
    int wnd_node_exists(void *model, char *name);
    //! private
    int wnd_reset_wdo_pointer(void *model);
    //! resume the current wanda simualtion until the given end time
    int wnd_resume_unsteady_until(void *model, float end_time);
    /*!
    \return pointer to character buffer that contains the error message
    */
    const char *wnd_get_last_error();
    //! upgrade model to latest file format specification
    int wnd_upgrade_model(void *model);
}
#endif
