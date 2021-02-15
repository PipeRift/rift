# CMake version
cmake_minimum_required (VERSION 3.12)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(RIFT_ENABLE_PROFILER "Should profiler recording be included in the build?" ON)

# Turn on the verbose
set(CMAKE_VERBOSE_MAKEFILE OFF)
# Export commands for Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Rift VERSION 0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CMake/DownloadProject.cmake)
include(Libs/Core/CMake/Util.cmake)
include(CMake/Util.cmake)

add_subdirectory(Extern)

# Framework
set(RIFT_BUILD_TESTS ON CACHE BOOL "Build RiftCore tests")
add_subdirectory(Libs/Core)
add_subdirectory(Libs/Framework)

# CLI
add_subdirectory(Apps/CLI)

# Editor
add_subdirectory(Apps/Editor)

# Tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)

    if(BUILD_TESTING)
        add_subdirectory(Tests)
    endif()
endif()

# clang-format
include(CMake/CheckClangTools.cmake)

# Additional targets to perform clang-format/clang-tidy
file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS
    Apps/**/*.cpp Apps/**/*.h
    Libs/Framework/**/*.cpp Libs/Framework/**/*.h
    Libs/Core/Include/**/*.cpp Libs/Core/Include/**/*.h
    Libs/Core/Source/**/*.cpp Libs/Core/Source/**/*.h
    Libs/Core/Tests/**/*.cpp Libs/Core/Tests/**/*.h
    Tests/**/*.cpp Tests/**/*.h
)

if(CLANG_FORMAT_EXE)
    add_custom_target(ClangFormat COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES})
    set_target_properties (ClangFormat PROPERTIES FOLDER Util)
endif()

if(CLANG_TIDY_EXE)
    add_custom_target(ClangTidy COMMAND ${CLANG_TIDY_EXE} -p ./ ${ALL_SOURCE_FILES} -checks=* -fix)
    set_target_properties (ClangTidy PROPERTIES FOLDER Util)
endif()
