

cmake_minimum_required(VERSION 3.1...3.28)

set(target_name "mdstat2collectd")

project(
  ${target_name}
  VERSION 1.0
  LANGUAGES C)

add_executable(${target_name} src/main.c src/mdstat_reader.c src/utils.c src/output_collectd.c)

target_include_directories(${target_name} PRIVATE src/)

target_compile_definitions(${target_name} PUBLIC -DDEFAULT_INTERVAL=60)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${target_name} PUBLIC -DMDSTAT_FILE="../mdstat.txt")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_definitions(${target_name} PUBLIC -DMDSTAT_FILE="/proc/mdstat")
else()
  target_compile_definitions(${target_name} PUBLIC -DMDSTAT_FILE="/proc/mdstat")
endif()

target_compile_options(${target_name} PRIVATE -Wall PRIVATE -Wextra PRIVATE -Wpedantic)
