
set(PIPE_BUILD_TESTS ${RIFT_BUILD_TESTS} CACHE BOOL "Build Pipe tests")
add_subdirectory(Pipe)

add_subdirectory(Framework)
add_subdirectory(Backends/LLVM)
add_subdirectory(Bindings/Native)

add_subdirectory(UI)
add_subdirectory(Views/Graph)
add_subdirectory(Editor)

# TODO: Do this automatically with rift_runtime_module/rift_editor_module
set(RIFT_RUNTIME_MODULES RiftBackendLLVM RiftViewGraph PARENT_SCOPE)
set(RIFT_EDITOR_MODULES RiftBindingNativeEditor RiftViewGraphEditor PARENT_SCOPE)