# CMake version
cmake_minimum_required (VERSION 3.12)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(VCLANG_ENABLE_PROFILER "Should profiler recording be included in the build?" ON)

# Turn on the verbose
set(CMAKE_VERBOSE_MAKEFILE OFF)
# Export commands for Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(VCLang VERSION 0.1 LANGUAGES CXX)

include(CMake/DownloadProject.cmake)
include(CMake/Util.cmake)

add_subdirectory(Extern)

# Framework
add_subdirectory(Libs/Core)
add_subdirectory(Libs/Framework)

# CLI
add_subdirectory(Apps/CLI)

# Editor
add_subdirectory(Apps/Editor)


# clang-format
include(CMake/CheckClangTools.cmake)

# Additional targets to perform clang-format/clang-tidy
file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS Apps/**/*.cpp Apps/**/*.h Libs/**/*.cpp Libs/**/*.h)

if(CLANG_FORMAT_EXE)
    add_custom_target(ClangFormat COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES})
endif()

if(CLANG_TIDY_EXE)
    add_custom_target(ClangTidy COMMAND ${CLANG_TIDY_EXE} -p ./ ${ALL_SOURCE_FILES} -checks=* -fix)
endif()
