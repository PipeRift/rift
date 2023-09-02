# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftEditor STATIC)

target_include_directories(RiftEditor PUBLIC Include)
file(GLOB_RECURSE RiftEditor_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftEditor PRIVATE ${RiftEditor_SOURCE_FILES})

target_link_libraries(RiftEditor PUBLIC
    RiftAST
    RiftBackendMIR
    RiftUI
    CLI11
)

rift_module(RiftEditor)
rift_enable_module_resources(RiftEditor)
set_icon(RiftEditor ../../Resources/Icon.ico)
