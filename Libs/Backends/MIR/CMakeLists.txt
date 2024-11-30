# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftBackendMIRLib STATIC)
target_link_libraries(RiftBackendMIRLib PUBLIC RiftASTLib)
target_link_libraries(RiftBackendMIRLib PRIVATE RiftBindingNativeLib mir)
rift_compiler_module(RiftBackendMIRLib)
