# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftBackendMIR STATIC)
rift_compiler_module(RiftBackendMIR)

target_link_libraries(RiftBackendMIR PUBLIC Rift)
target_link_libraries(RiftBackendMIR PRIVATE
    mir_static
    RiftBindingNative
)

