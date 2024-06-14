# Cmake build script for Wanda API  (master branch  v27-5-2024)
cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(wandaapi STATIC 
src/c_wanda_engine.cpp
src/c_wanda_model.cpp
src/calc_hcs.cpp
src/deltares_helper_functions.cpp
src/nefis_file.cpp
src/Wanda_engine.cpp
src/wanda_item.cpp
src/wanda_table.cpp
src/Wandacomponent.cpp
src/Wandadef.cpp
src/Wandamodel.cpp
src/Wandanode.cpp
src/Wandaproperty.cpp
src/Wandasigline.cpp
)

target_compile_definitions(wandaapi PRIVATE WANDAMODEL_EXPORT /std:c++latest /permissive- /W4 /w14640)
target_compile_definitions(wandaapi PUBLIC WANDAMODEL_EXPORT)

#include paths needed
target_include_directories(wandaapi PUBLIC  
"${CMAKE_CURRENT_SOURCE_DIR}/include/"
"${CMAKE_CURRENT_SOURCE_DIR}/lib/Dauth/include/"
)

## Dependency library setup. needed for copying dlls to the build directory
add_library(nefis SHARED IMPORTED)
set_target_properties(nefis PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/nefis/nefis.dll"
  IMPORTED_IMPLIB   "${CMAKE_CURRENT_SOURCE_DIR}/lib/nefis/nefis_dll.lib"
)

add_library(dauth_debug SHARED IMPORTED)
set_target_properties(dauth_debug PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/dauth/debug/dauth.dll"
  IMPORTED_IMPLIB   "${CMAKE_CURRENT_SOURCE_DIR}/lib/dauth/debug/dauth.lib"
)

add_library(dauth_release SHARED IMPORTED)
set_target_properties(dauth_release PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/dauth/release/dauth.dll"
  IMPORTED_IMPLIB   "${CMAKE_CURRENT_SOURCE_DIR}/lib/dauth/release/dauth.lib"
)


#General lib dependencies
target_link_libraries( wandaapi PUBLIC 
  nefis
  "version.lib"
)
# Debug/Release libs
target_link_libraries( wandaapi PUBLIC 
debug dauth_debug
optimized dauth_release
)
target_link_libraries( wandaapi PRIVATE 
debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/dauth/debug/dauth_client.lib"
optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/dauth/release/dauth_client.lib"
)

