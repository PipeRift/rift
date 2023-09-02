# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftViewGraph STATIC)
rift_compiler_module(RiftViewGraph)
target_link_libraries(RiftViewGraph PUBLIC RiftAST)

add_library(RiftViewGraphEditor STATIC)
rift_editor_module(RiftViewGraphEditor)
target_link_libraries(RiftViewGraphEditor PUBLIC RiftEditor RiftViewGraph)
