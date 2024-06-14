add_executable(mgwso main.cpp)

target_include_directories(mgwso PRIVATE 
  "${CMAKE_BINARY_DIR}/configured_files/include"
  "$<TARGET_PROPERTY:wandaapi,INTERFACE_INCLUDE_DIRECTORIES>")

target_link_libraries(
  mgwso
  PRIVATE mgwso::mgwso_options
          mgwso::mgwso_warnings
          wandaapi)

target_link_system_libraries(
  mgwso
  PRIVATE
          CLI11::CLI11
          fmt::fmt
          spdlog::spdlog)

target_include_directories(mgwso PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
if (WIN32)
  add_custom_command(
    TARGET mgwso POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:mgwso> $<TARGET_FILE_DIR:mgwso>
    COMMAND_EXPAND_LISTS
  )
endif()