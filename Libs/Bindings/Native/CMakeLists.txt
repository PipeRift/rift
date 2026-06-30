# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftBindingNativeLib STATIC)


target_link_libraries(RiftBindingNativeLib PUBLIC RiftASTLib)

rift_compiler_module(RiftBindingNativeLib)

add_library(RiftBindingNativeEditorLib STATIC)
rift_editor_module(RiftBindingNativeEditorLib)
target_link_libraries(RiftBindingNativeEditorLib PUBLIC RiftEditorLib RiftBindingNativeLib)
