# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftEditorLib STATIC)

target_include_directories(RiftEditorLib PUBLIC Include)
file(GLOB_RECURSE RiftEditorLib_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftEditorLib PRIVATE ${RiftEditorLib_SOURCE_FILES})

target_link_libraries(RiftEditorLib PUBLIC
    RiftASTLib
    RiftBackendMIRLib
    RiftUILib
    CLI11
)

rift_module(RiftEditorLib)
rift_enable_module_resources(RiftEditorLib)
set_target_properties(RiftEditorLib PROPERTIES OUTPUT_NAME "RiftEditor")
