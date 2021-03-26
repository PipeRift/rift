# CMake version
cmake_minimum_required (VERSION 3.12)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


# Turn on the verbose
set(CMAKE_VERBOSE_MAKEFILE OFF)
# Export commands for Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Rift VERSION 0.1 LANGUAGES CXX C)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(RIFT_IS_PROJECT ON)
else()
    set(RIFT_IS_PROJECT OFF)
endif()
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(RIFT_BUILD_TESTS "Build Rift and Core tests" ${RIFT_IS_PROJECT})


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CMake/DownloadProject.cmake)
include(Libs/Core/CMake/Util.cmake)
include(CMake/Util.cmake)

add_subdirectory(Extern)

# Framework
set(RIFT_CORE_BUILD_TESTS ${RIFT_BUILD_TESTS} CACHE BOOL "Build RiftCore tests")
add_subdirectory(Libs/Core)
add_subdirectory(Libs/Framework)

# CLI
add_subdirectory(Apps/CLI)

# Editor
add_subdirectory(Libs/UI)
add_subdirectory(Apps/Editor)

# Resources folder
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/Bin/Resources)
file(COPY Resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Bin)

# Tests
if(BUILD_TESTING AND RIFT_BUILD_TESTS)
    include(CTest)
    add_subdirectory(Tests)
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
    add_custom_target(ClangTidy COMMAND ${CLANG_TIDY_EXE} -p=./Build ${ALL_SOURCE_FILES} --fix)
    set_target_properties (ClangTidy PROPERTIES FOLDER Util)
endif()
