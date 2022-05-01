
set(RIFT_LLVM_PATH_RELEASE "" CACHE STRING "Location of Rift LLVM installation to use on Release. CMake will try to find if this path is not set")
set(RIFT_LLVM_PATH_DEBUG "" CACHE STRING "Location of Rift LLVM installation to use on Debug. CMake will try to find if this path is not set")
set(RIFT_LLVM_PATH_MINSIZEREL "" CACHE STRING "Location of Rift LLVM installation to use on MinSizeRel. CMake will try to find if this path is not set")
set(RIFT_LLVM_PATH_RELWITHDEBINFO "" CACHE STRING "Location of Rift LLVM installation to use on RelWithDebInfo. CMake will try to find if this path is not set")
set(RIFT_LLVM_PATH "" CACHE STRING "Location of an external LLVM installation to use. Used if RIFT_LLVM_PATH_{CONFIG} is not set")

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPER)
if (NOT ${RIFT_LLVM_PATH_${CMAKE_BUILD_TYPE_UPPER}} STREQUAL "")
    set(RIFT_LLVM_PATH ${RIFT_LLVM_PATH_${CMAKE_BUILD_TYPE_UPPER}})
else()
    message(STATUS "RIFT_LLVM_PATH_${CMAKE_BUILD_TYPE_UPPER} not provided. Defaulted to RIFT_LLVM_PATH")
endif()

if (NOT RIFT_LLVM_PATH STREQUAL "")
    message(STATUS "Provided LLVM installation: ${RIFT_LLVM_PATH}")
    set(LLVM_DIR "${RIFT_LLVM_PATH}/lib/cmake/llvm")
else()
    message(WARNING "LLVM path not provided. Set RIFT_LLVM_PATH_{CONFIG} to your llvm installation matching the config mode")
endif()

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
message(STATUS "LLVM_LIBS: ${LLVM_AVAILABLE_LIBS}")
message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM_DEFINITIONS: ${LLVM_DEFINITIONS_LIST}")


add_library(RiftLLVM INTERFACE)
target_include_directories(RiftLLVM INTERFACE ${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs core x86asmparser x86codegen)
target_link_libraries(RiftLLVM INTERFACE ${LLVM_AVAILABLE_LIBS})
target_compile_definitions(RiftLLVM INTERFACE ${LLVM_DEFINITIONS_LIST}  -DNOMINMAX)
# rift_target_disable_all_warnings(LLVM INTERFACE)

