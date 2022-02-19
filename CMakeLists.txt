# CMake version
cmake_minimum_required (VERSION 3.21)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


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
option(RIFT_BUILD_TESTS "Build Rift and Core tests" ${RIFT_IS_PROJECT})


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CMake/DownloadProject.cmake)
include(Libs/Core/CMake/Util.cmake)
include(CMake/Util.cmake)
include(CMake/SetIcon.cmake)
include(CMake/CheckClangTools.cmake)


# Add LLVM
set(RIFT_LLVM_BUILD_PATH "" CACHE STRING "Location of an external LLVM installation to use. CMake will try to find if this path is not set")
if (NOT RIFT_LLVM_BUILD_PATH STREQUAL "")
    message(STATUS "Provided LLVM installation: ${RIFT_LLVM_BUILD_PATH}")
    set(LLVM_DIR "${RIFT_LLVM_BUILD_PATH}/lib/cmake/llvm")
endif()
find_package(LLVM CONFIG REQUIRED)
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_library(LLVM INTERFACE)
message(STATUS "LLVM_LIBS: ${LLVM_AVAILABLE_LIBS}")
message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM_DEFINITIONS: ${LLVM_DEFINITIONS_LIST}")
target_include_directories(LLVM INTERFACE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(LLVM INTERFACE ${LLVM_DEFINITIONS_LIST})
llvm_map_components_to_libnames(llvm_libs core x86asmparser x86codegen)
target_link_libraries(LLVM INTERFACE ${llvm_libs})


add_subdirectory(Extern)

add_subdirectory(Libs)

# Executables
add_subdirectory(Apps/CLI)
add_subdirectory(Apps/Editor)

# Resources folder
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/Bin/Resources)
file(COPY Resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Bin)

# Tests
if(BUILD_TESTING AND RIFT_BUILD_TESTS)
    include(CTest)
    add_subdirectory(Tests)
    add_subdirectory(Benchmarks)
endif()

# clang-format
if(CLANG_FORMAT_EXE)
    # Additional targets to perform clang-format/clang-tidy
    file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS
        Apps/**/*.cpp Apps/**/*.h
        Libs/Framework/**/*.cpp Libs/Framework/**/*.h
        Libs/UI/**/*.cpp Libs/UI/**/*.h
        Libs/Core/Include/**/*.cpp Libs/Core/Include/**/*.h
        Libs/Core/Src/**/*.cpp Libs/Core/Src/**/*.h
        Libs/Core/Tests/**/*.cpp Libs/Core/Tests/**/*.h
        Tests/**/*.cpp Tests/**/*.h
    )

    add_custom_target(ClangFormat COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES})
    set_target_properties (ClangFormat PROPERTIES FOLDER Util)
endif()

if(CLANG_TIDY_EXE)
    file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS
        Apps/**/*.cpp
        Libs/Framework/**/*.cpp
        Libs/UI/**/*.cpp
        Libs/Core/Include/**/*.cpp
        Libs/Core/Src/**/*.cpp
    )

    add_custom_target(ClangTidy COMMAND ${DO_CLANG_TIDY} --fix-errors --fix-notes ${ALL_SOURCE_FILES})
    set_target_properties (ClangTidy PROPERTIES FOLDER Util)
endif()
