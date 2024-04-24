add_executable(mgwso main.cpp)

target_link_libraries(
  mgwso
  PRIVATE mgwso::mgwso_options
          mgwso::mgwso_warnings)

target_link_system_libraries(
  mgwso
  PRIVATE
          CLI11::CLI11
          fmt::fmt
          spdlog::spdlog)

target_include_directories(mgwso PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")