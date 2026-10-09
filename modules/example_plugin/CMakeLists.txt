cmake_minimum_required(VERSION 3.21)

# ---------------------------------------------------------------------------
#  Example ASMLEX plugin.
#
#  Builds a DLL exporting `asmlex_plugin_entry`.  Copy this directory to start
#  a new plugin, then rename `example_plugin` below.
# ---------------------------------------------------------------------------

project(asmlex_example_plugin VERSION 1.0.0 LANGUAGES CXX)

add_library(example_plugin SHARED example_plugin.cpp)

target_include_directories(example_plugin PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

set_target_properties(example_plugin PROPERTIES
  PREFIX ""
  OUTPUT_NAME "asmlex_example_plugin")

if(MSVC)
  target_compile_options(example_plugin PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(example_plugin PRIVATE -Wall -Wextra)
endif()
