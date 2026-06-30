# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftViewGraphLib STATIC)
rift_compiler_module(RiftViewGraphLib)
target_link_libraries(RiftViewGraphLib PUBLIC RiftASTLib)

add_library(RiftViewGraphEditorLib STATIC)
rift_editor_module(RiftViewGraphEditorLib)
target_link_libraries(RiftViewGraphEditorLib PUBLIC RiftEditorLib RiftViewGraphLib)
