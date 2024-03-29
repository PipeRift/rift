
set(RIFT_DEFAULT_TO_INTERNAL_LLVM ${PLATFORM_WINDOWS} CACHE STRING "If true, use internal llvm build path. Default: true in Windows, false in Linux")
set(RIFT_INTERNAL_LLVM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Extern/rift-llvm")

set(RIFT_LLVM_PATH_RELEASE "" CACHE STRING "Location of Rift LLVM installation to use on Release. CMake will try to find if this path is not set")
set(RIFT_LLVM_PATH_DEBUG "" CACHE STRING "Location of Rift LLVM installation to use on Debug. CMake will try to find if this path is not set")
set(RIFT_LLVM_PATH_MINSIZEREL "" CACHE STRING "Location of Rift LLVM installation to use on MinSizeRel. CMake will try to find if this path is not set")
set(RIFT_LLVM_PATH_RELWITHDEBINFO "" CACHE STRING "Location of Rift LLVM installation to use on RelWithDebInfo. CMake will try to find if this path is not set")
set(RIFT_LLVM_PATH "" CACHE STRING "Location of an external LLVM installation to use. Used if RIFT_LLVM_PATH_{CONFIG} is not set. If left empty, INTERNAL_RIFT_LLVM_PATH is used.")

# Check if specific build type path is required
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPER)
if (RIFT_LLVM_PATH STREQUAL "" AND NOT ${RIFT_LLVM_PATH_${CMAKE_BUILD_TYPE_UPPER}} STREQUAL "")
    message(STATUS "Using RIFT_LLVM_PATH_${CMAKE_BUILD_TYPE_UPPER}")
    set(RIFT_LLVM_PATH ${RIFT_LLVM_PATH_${CMAKE_BUILD_TYPE_UPPER}})
endif()

# Check if RIFT_LLVM_PATH should be used
if (NOT RIFT_LLVM_PATH STREQUAL "")
    message(STATUS "Provided explicit LLVM path: ${RIFT_LLVM_PATH}")
    set(RIFT_LLVM_BIN_PATH ${RIFT_LLVM_PATH})
elseif (RIFT_DEFAULT_TO_INTERNAL_LLVM AND NOT RIFT_INTERNAL_LLVM_PATH STREQUAL "")
    set(RIFT_LLVM_PATH ${RIFT_INTERNAL_LLVM_PATH}/build)
    message(STATUS "Provided internal LLVM path: ${RIFT_LLVM_PATH}")
    set(RIFT_LLVM_BIN_PATH ${RIFT_LLVM_PATH}/${CMAKE_BUILD_TYPE})
else()
    message(STATUS "LLVM path not provided. Will be searched in the system")
    set(RIFT_LLVM_BIN_PATH ${RIFT_LLVM_PATH})
endif()

set(LLVM_DIR "${RIFT_LLVM_PATH}/lib/cmake/llvm")
set(Clang_DIR "${RIFT_LLVM_PATH}/lib/cmake/clang")

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

find_package(Clang REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${CLANG_CMAKE_DIR}")
include(AddClang)
message(STATUS "Found CLANG ${CLANG_PACKAGE_VERSION}")
message(STATUS "Using CLANGConfig.cmake in: ${Clang_DIR}")

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
message(STATUS "LLVM_LIBS: ${LLVM_AVAILABLE_LIBS}")
message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
# Copyright 2015-2023 Piperift - All rights reserved

message(STATUS "LLVM_DEFINITIONS: ${LLVM_DEFINITIONS_LIST}")
message(STATUS "CLANG_LIBS: ${CLANG_EXPORTED_TARGETS}")


add_library(RiftLLVM INTERFACE)
target_include_directories(RiftLLVM INTERFACE ${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs core support x86asmparser x86codegen)
target_link_libraries(RiftLLVM INTERFACE ${llvm_libs})
target_compile_definitions(RiftLLVM INTERFACE ${LLVM_DEFINITIONS_LIST} -DNOMINMAX)
#if(COMPILER_CLANG)
    #target_compile_options(RiftLLVM INTERFACE -fms-compatibility-version=14.20)
#endif()
pipe_target_disable_all_warnings(RiftLLVM INTERFACE)

add_library(RiftClang INTERFACE)
target_include_directories(RiftClang INTERFACE ${CLANG_INCLUDE_DIRS})
target_link_libraries(RiftClang INTERFACE libclang)
target_compile_definitions(RiftClang INTERFACE ${CLANG_DEFINITIONS_LIST} -DNOMINMAX)
target_link_libraries(RiftClang INTERFACE RiftLLVM)
pipe_target_disable_all_warnings(RiftClang INTERFACE)
