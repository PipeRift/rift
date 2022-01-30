
set(RIFT_CORE_BUILD_TESTS ${RIFT_BUILD_TESTS} CACHE BOOL "Build RiftCore tests")
add_subdirectory(Core)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Framework)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Backends/LLVM)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Backends/Cpp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/UI)
