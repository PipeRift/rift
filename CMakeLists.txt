# Copyright 2015-2023 Piperift - All rights reserved

# CMake version
cmake_minimum_required (VERSION 3.26)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# These targets accumulate compiler and editor modules for executables to link to
add_library(RiftCompilerModules INTERFACE)
add_library(RiftEditorModules INTERFACE)
add_library(RiftRuntimeModules INTERFACE)


################################################################################
# Project

project(Rift VERSION 0.1 LANGUAGES CXX C)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(RIFT_IS_PROJECT ON)
else()
    set(RIFT_IS_PROJECT OFF)
endif()
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(RIFT_BUILD_EDITOR "Build Editor" ON)
option(RIFT_BUILD_TESTS "Build Rift and Core tests" ${RIFT_IS_PROJECT})


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CMake/DownloadProject.cmake)
include(Extern/Pipe/CMake/Util.cmake)
include(CMake/Util.cmake)
include(CMake/SetIcon.cmake)
include(CMake/CheckClangTools.cmake)

add_subdirectory(Extern)

add_subdirectory(Libs)

# Executables
add_subdirectory(Apps/CLI)
if (RIFT_BUILD_EDITOR)
    add_subdirectory(Apps/Editor)
endif()

# Tests
if(BUILD_TESTING AND RIFT_BUILD_TESTS)
    include(CTest)
    add_subdirectory(Tests)
endif()

# clang-format
if(CLANG_FORMAT_EXE)
    # Additional targets to perform clang-format/clang-tidy
    file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS
        Apps/**/*.cpp Apps/**/*.h
        Libs/AST/**/*.cpp Libs/AST/**/*.h
        Libs/UI/**/*.cpp Libs/UI/**/*.h
        Libs/Pipe/Include/**/*.cpp Libs/Pipe/Include/**/*.h
        Libs/Pipe/Src/**/*.cpp Libs/Pipe/Src/**/*.h
        Libs/Pipe/Tests/**/*.cpp Libs/Pipe/Tests/**/*.h
        Tests/**/*.cpp Tests/**/*.h
    )

    add_custom_target(ClangFormat COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES})
    set_target_properties (ClangFormat PROPERTIES FOLDER Util)
endif()

if(CLANG_TIDY_EXE)
    file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS
        Apps/**/*.cpp
        Libs/AST/**/*.cpp
        Libs/UI/**/*.cpp
        Libs/Pipe/Include/**/*.cpp
        Libs/Pipe/Src/**/*.cpp
    )

    add_custom_target(ClangTidy COMMAND ${DO_CLANG_TIDY} --fix-errors --fix-notes ${ALL_SOURCE_FILES})
    set_target_properties (ClangTidy PROPERTIES FOLDER Util)
endif()
