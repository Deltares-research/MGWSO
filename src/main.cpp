
#include<optional>
#include<string>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>


// This file will be generated automatically when cur_you run the CMake
// configuration step. It creates a namespace called `mgwso`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>
#include "config.h"
#include "utility.h"
#include "wanda.h"

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv)
{
  try {
    CLI::App app{ fmt::format("{} version {}", mgwso::cmake::project_name, mgwso::cmake::project_version) };

    spdlog::info("Mooi-Goo Wanda Seawat OpenDA");
  }
  catch (const std::exception &e) {
    spdlog::error("Unhandled exception in main: {}", e.what());
  }
  if (argc != 2)
  {
    spdlog::error("Number of input argument is incorrect, only one argument should be supplied.");
    throw std::invalid_argument("Number of input argument is incorrect, only one argument should be supplied.");
  }
  
  try {
    std::string const input_file = argv[1];
    spdlog::info("Loading ini file");
    config config_data(get_exe_path(), input_file);

    spdlog::info("Deleting old output file");
    delete_output_file(config_data.get_model_path());
    spdlog::info("Loading Wanda model");
    wanda_model model(config_data.get_model_path(), config_data.wanda_bin);
    spdlog::info("Preparing WANDA model");
    prepare_wanda_model(config_data, model);
    spdlog::info("Running WANDA model");
    run_wanda_model(model);
    model.close();
  } 
  catch (const std::exception &e) {
    spdlog::error("Unhandled exception in main: {}", e.what());
  }

  
  return EXIT_SUCCESS;
}