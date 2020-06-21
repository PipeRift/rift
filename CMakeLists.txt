# CMake version
cmake_minimum_required (VERSION 3.13)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

option(ENABLE_PROFILER "Should profiler recording be included in the build?" ON)

# Turn on the verbose
set(CMAKE_VERBOSE_MAKEFILE OFF)
# Export commands for Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(VCLang VERSION 0.1 LANGUAGES CXX)

include(CMake/conan.cmake)
include(CMake/DownloadProject.cmake)
include(CMake/Util.cmake)

#CONAN PACKAGES
conan_cmake_run(REQUIRES eastl/3.16.05 BASIC_SETUP CMAKE_TARGETS BUILD missing)
conan_cmake_run(REQUIRES glm/0.9.9.8 BASIC_SETUP CMAKE_TARGETS BUILD missing)
conan_cmake_run(REQUIRES spdlog/1.6.1 BASIC_SETUP CMAKE_TARGETS BUILD missing)
conan_cmake_run(REQUIRES nlohmann_json/3.8.0 BASIC_SETUP CMAKE_TARGETS BUILD missing)
conan_cmake_run(REQUIRES date/2.4.1 BASIC_SETUP CMAKE_TARGETS BUILD missing)

add_subdirectory(Extern)

# Framework
add_subdirectory(Libs/Core)
add_subdirectory(Libs/Framework)

# CLI
add_subdirectory(Apps/CLI)

# Editor
add_subdirectory(Apps/Editor)
