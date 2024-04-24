cmake_minimum_required(VERSION 3.21)

project(CmakeConfigPackageTests LANGUAGES CXX)

# ---- Test as standalone project the exported config package ----

if(PROJECT_IS_TOP_LEVEL OR TEST_INSTALLED_VERSION)
  enable_testing()

  find_package(mgwso CONFIG REQUIRED) # for intro, project_options, ...

  if(NOT TARGET mgwso_options)
    message(FATAL_ERROR "Required config package not found!")
    return() # be strictly paranoid for Template Janitor github action! CK
  endif()
endif()

# ---- Dependencies ----

include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

# Provide a simple smoke test to make sure that the CLI works and can display a --help message
add_test(NAME cli.has_help COMMAND mgwso --help)
# Provide a test to verify that the version being reported from the application
# matches the version given to CMake. This will be important once you package
# your program. Real world shows that this is the kind of simple mistake that is easy
# to make, but also easy to test for.
add_test(NAME cli.version_matches COMMAND mgwso --version)
set_tests_properties(cli.version_matches PROPERTIES PASS_REGULAR_EXPRESSION "${PROJECT_VERSION}")

add_executable(tests tests.cpp)
target_link_libraries(
  tests
  PRIVATE mgwso::mgwso_warnings
          mgwso::mgwso_options
          Catch2::Catch2WithMain)

catch_discover_tests(
                        tests
                        TEST_PREFIX
                        "unittests."
                        REPORTER
                        XML
                        OUTPUT_DIR
                        .
                        OUTPUT_PREFIX
                        "unittests."
                        OUTPUT_SUFFIX
                        .xml)          