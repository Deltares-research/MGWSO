include(cmake/SystemLink.cmake)
# include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(mgwso_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(mgwso_setup_options)
  option(mgwso_ENABLE_HARDENING "Enable hardening" ON)
  option(mgwso_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    mgwso_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    mgwso_ENABLE_HARDENING
    OFF)

  mgwso_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR mgwso_PACKAGING_MAINTAINER_MODE)
    option(mgwso_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(mgwso_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(mgwso_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(mgwso_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(mgwso_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(mgwso_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(mgwso_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(mgwso_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(mgwso_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(mgwso_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(mgwso_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(mgwso_ENABLE_PCH "Enable precompiled headers" OFF)
    option(mgwso_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(mgwso_ENABLE_IPO "Enable IPO/LTO" ON)
    option(mgwso_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(mgwso_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(mgwso_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(mgwso_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(mgwso_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(mgwso_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(mgwso_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(mgwso_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(mgwso_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(mgwso_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(mgwso_ENABLE_PCH "Enable precompiled headers" OFF)
    option(mgwso_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      mgwso_ENABLE_IPO
      mgwso_WARNINGS_AS_ERRORS
      mgwso_ENABLE_USER_LINKER
      mgwso_ENABLE_SANITIZER_ADDRESS
      mgwso_ENABLE_SANITIZER_LEAK
      mgwso_ENABLE_SANITIZER_UNDEFINED
      mgwso_ENABLE_SANITIZER_THREAD
      mgwso_ENABLE_SANITIZER_MEMORY
      mgwso_ENABLE_UNITY_BUILD
      mgwso_ENABLE_CLANG_TIDY
      mgwso_ENABLE_CPPCHECK
      mgwso_ENABLE_COVERAGE
      mgwso_ENABLE_PCH
      mgwso_ENABLE_CACHE)
  endif()

  # mgwso_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  # if(LIBFUZZER_SUPPORTED AND (mgwso_ENABLE_SANITIZER_ADDRESS OR mgwso_ENABLE_SANITIZER_THREAD OR mgwso_ENABLE_SANITIZER_UNDEFINED))
  #   set(DEFAULT_FUZZER ON)
  # else()
  #   set(DEFAULT_FUZZER OFF)
  # endif()

  # option(mgwso_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(mgwso_global_options)
  if(mgwso_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    mgwso_enable_ipo()
  endif()

  mgwso_supports_sanitizers()

  # if(mgwso_ENABLE_HARDENING AND mgwso_ENABLE_GLOBAL_HARDENING)
  #   include(cmake/Hardening.cmake)
  #   if(NOT SUPPORTS_UBSAN 
  #      OR mgwso_ENABLE_SANITIZER_UNDEFINED
  #      OR mgwso_ENABLE_SANITIZER_ADDRESS
  #      OR mgwso_ENABLE_SANITIZER_THREAD
  #      OR mgwso_ENABLE_SANITIZER_LEAK)
  #     set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
  #   else()
  #     set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
  #   endif()
  #   message("${mgwso_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${mgwso_ENABLE_SANITIZER_UNDEFINED}")
  #   mgwso_enable_hardening(mgwso_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  # endif()
endmacro()

macro(mgwso_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(mgwso_warnings INTERFACE)
  add_library(mgwso_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  mgwso_set_project_warnings(
    mgwso_warnings
    ${mgwso_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(mgwso_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    mgwso_configure_linker(mgwso_options)
  endif()

  include(cmake/Sanitizers.cmake)
  mgwso_enable_sanitizers(
    mgwso_options
    ${mgwso_ENABLE_SANITIZER_ADDRESS}
    ${mgwso_ENABLE_SANITIZER_LEAK}
    ${mgwso_ENABLE_SANITIZER_UNDEFINED}
    ${mgwso_ENABLE_SANITIZER_THREAD}
    ${mgwso_ENABLE_SANITIZER_MEMORY})

  set_target_properties(mgwso_options PROPERTIES UNITY_BUILD ${mgwso_ENABLE_UNITY_BUILD})

  if(mgwso_ENABLE_PCH)
    target_precompile_headers(
      mgwso_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  # if(mgwso_ENABLE_CACHE)
  #   include(cmake/Cache.cmake)
  #   mgwso_enable_cache()
  # endif()

  include(cmake/StaticAnalyzers.cmake)
  if(mgwso_ENABLE_CLANG_TIDY)
    mgwso_enable_clang_tidy(mgwso_options ${mgwso_WARNINGS_AS_ERRORS})
  endif()

  if(mgwso_ENABLE_CPPCHECK)
    mgwso_enable_cppcheck(${mgwso_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(mgwso_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    mgwso_enable_coverage(mgwso_options)
  endif()

  if(mgwso_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(mgwso_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  # if(mgwso_ENABLE_HARDENING AND NOT mgwso_ENABLE_GLOBAL_HARDENING)
  #   include(cmake/Hardening.cmake)
  #   if(NOT SUPPORTS_UBSAN 
  #      OR mgwso_ENABLE_SANITIZER_UNDEFINED
  #      OR mgwso_ENABLE_SANITIZER_ADDRESS
  #      OR mgwso_ENABLE_SANITIZER_THREAD
  #      OR mgwso_ENABLE_SANITIZER_LEAK)
  #     set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
  #   else()
  #     set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
  #   endif()
  #   mgwso_enable_hardening(mgwso_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  # endif()

endmacro()