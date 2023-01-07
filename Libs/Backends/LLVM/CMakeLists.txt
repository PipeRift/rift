# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftBackendLLVM STATIC)
rift_compiler_module(RiftBackendLLVM)
rift_enable_module_resources(RiftBackendLLVM)

target_link_libraries(RiftBackendLLVM PUBLIC Rift)
target_link_libraries(RiftBackendLLVM PRIVATE
    RiftLLVM
    RiftBindingNative
)
target_compile_definitions(RiftBackendLLVM PRIVATE RIFT_LLVM_LINKER="${RIFT_LLVM_LINKER}" RIFT_LLVM_LINKER_PATH="${RIFT_LLVM_LINKER_PATH}")

install(TARGETS RiftBackendLLVM
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
