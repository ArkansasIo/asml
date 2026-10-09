cmake_minimum_required(VERSION 3.21)

# ---------------------------------------------------------------------------
#  ASMLEX - Assembler / Disassembler Studio
#
#  A Windows desktop IDE for writing, assembling, disassembling, running and
#  debugging low level assembly, built on a single C++23 code base.
# ---------------------------------------------------------------------------

project(asmlex
        VERSION 1.0.0
        DESCRIPTION "ASMLEX Assembler & Disassembler Studio"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

if(MSVC)
  add_compile_options(/utf-8 /permissive- /Zc:__cplusplus /Zc:preprocessor
                      /W4 /wd4100 /wd4244 /wd4267 /MP)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS UNICODE _UNICODE NOMINMAX
                          WIN32_LEAN_AND_MEAN ASMLEX_PLATFORM_WINDOWS=1)
  add_link_options(/SUBSYSTEM:WINDOWS)
else()
  add_compile_options(-Wall -Wextra -Wno-unused-parameter)
  add_compile_definitions(UNICODE _UNICODE NOMINMAX ASMLEX_PLATFORM_WINDOWS=1
                          ASMLEX_NO_MSVC=1)
  add_link_options(-mwindows)
endif()

set(ASMLEX_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# --- Vendored jsoncpp -------------------------------------------------------
add_library(jsoncpp STATIC
  ${ASMLEX_ROOT}/third_party/jsoncpp/json_reader.cpp
  ${ASMLEX_ROOT}/third_party/jsoncpp/json_value.cpp
  ${ASMLEX_ROOT}/third_party/jsoncpp/json_writer.cpp)
target_include_directories(jsoncpp PUBLIC ${ASMLEX_ROOT}/third_party/jsoncpp)
if(MSVC)
  target_compile_options(jsoncpp PRIVATE /W0 /wd4996)
endif()

# --- ASMLEX core static library --------------------------------------------
file(GLOB_RECURSE ASMLEX_CORE_SOURCES CONFIGURE_DEPENDS
     ${ASMLEX_ROOT}/src/*.cpp)
list(FILTER ASMLEX_CORE_SOURCES EXCLUDE REGEX "main\\.cpp$")

add_library(asmlex_core STATIC ${ASMLEX_CORE_SOURCES})
target_include_directories(asmlex_core
  PUBLIC  ${ASMLEX_ROOT}/include
  PRIVATE ${ASMLEX_ROOT}/src)
target_link_libraries(asmlex_core PUBLIC jsoncpp)

add_executable(asmlex WIN32 ${ASMLEX_ROOT}/src/main.cpp)
target_link_libraries(asmlex PRIVATE asmlex_core)

# Mirror every header next to the module tree for convenience when browsing.
source_group(TREE ${ASMLEX_ROOT} FILES ${ASMLEX_CORE_SOURCES})

set_target_properties(asmlex PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)

message(STATUS "ASMLEX: C++23 build configured (${CMAKE_CXX_COMPILER_ID}).")
