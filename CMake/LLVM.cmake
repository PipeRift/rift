
set(RIFT_LLVM_PATH "" CACHE STRING "Location of an external LLVM installation to use. CMake will try to find if this path is not set")
if (NOT RIFT_LLVM_PATH STREQUAL "")
    message(STATUS "Provided LLVM installation: ${RIFT_LLVM_PATH}")
    set(LLVM_DIR "${RIFT_LLVM_PATH}/lib/cmake/llvm")
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


add_library(LLVM INTERFACE)
target_include_directories(LLVM INTERFACE ${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs core x86asmparser x86codegen)
target_link_libraries(LLVM INTERFACE ${LLVM_AVAILABLE_LIBS})
target_compile_definitions(LLVM INTERFACE ${LLVM_DEFINITIONS_LIST}  -DNOMINMAX)
# rift_target_disable_all_warnings(LLVM INTERFACE)

