# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftBindingNative STATIC)


target_link_libraries(RiftBindingNative PUBLIC RiftAST)
target_link_libraries(RiftBindingNative PRIVATE RiftClang)

rift_compiler_module(RiftBindingNative)

add_library(RiftBindingNativeEditor STATIC)
rift_editor_module(RiftBindingNativeEditor)
target_link_libraries(RiftBindingNativeEditor PUBLIC RiftEditor RiftBindingNative)
