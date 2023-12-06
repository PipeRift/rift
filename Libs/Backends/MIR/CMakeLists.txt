# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftBackendMIR STATIC)
target_link_libraries(RiftBackendMIR PUBLIC RiftAST)
target_link_libraries(RiftBackendMIR PRIVATE RiftBindingNative mir)
rift_compiler_module(RiftBackendMIR)

