
set(RIFT_CORE_BUILD_TESTS ${RIFT_BUILD_TESTS} CACHE BOOL "Build RiftCore tests")
add_subdirectory(Core)
add_subdirectory(Framework)
add_subdirectory(Backends/LLVM)
add_subdirectory(Backends/Cpp)
add_subdirectory(UI)
