
#include<optional>
#include<string>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>
#include <wandamodel.h>


// This file will be generated automatically when cur_you run the CMake
// configuration step. It creates a namespace called `mgwso`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>


// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv)
{
  try {
    CLI::App app{ fmt::format("{} version {}", mgwso::cmake::project_name, mgwso::cmake::project_version) };

    std::optional<std::string> config_path;
    app.add_option("-f,--file", config_path, "Configuration file path");
    std::optional<std::string> model_path;
    app.add_option("-m,--model", model_path, "model file path");
    bool show_version = false;
    app.add_flag("--version", show_version, "Show version information");

    CLI11_PARSE(app, argc, argv);

    if (show_version) {
      fmt::print("{}\n", mgwso::cmake::project_version);
      return EXIT_SUCCESS;
    }
    spdlog::info("Mooi-Goo Wanda Seawat OpenDA");
    spdlog::info("Configuration file: {}", config_path.value_or("empty"));
    spdlog::info("Model file: {}", model_path.value_or("not provided"));
  } 
  catch (const std::exception &e) {
    spdlog::error("Unhandled exception in main: {}", e.what());
  }

  
  try {
    auto model = wanda_model("test.wdi", "c:\\Program Files (x86)\\Deltares\\Wanda 4.7\\Bin\\");
    for(const auto& item: model.get_all_pipes()){
      spdlog::info("Pipe: {}", item->get_complete_name_spec());
    }
    model.close();
  } 
  catch (const std::exception &e) {
    spdlog::error("Unhandled exception in main: {}", e.what());
  }

  
  return EXIT_SUCCESS;
}