# CMake version
cmake_minimum_required (VERSION 3.12)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

option(ENABLE_PROFILER "Should profiler recording be included in the build?" ON)

# Turn on the verbose
set(CMAKE_VERBOSE_MAKEFILE OFF)
# Export commands for Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project(VCLang VERSION 0.1 LANGUAGES CXX)

include(CMake/conan.cmake)
include(CMake/DownloadProject.cmake)
include(CMake/Util.cmake)

add_subdirectory(Extern/Tracy)

# Framework
add_subdirectory(Libs/Core)
add_subdirectory(Libs/Framework)

# CLI
add_subdirectory(Apps/CLI)

# Editor
add_subdirectory(Apps/Editor)
