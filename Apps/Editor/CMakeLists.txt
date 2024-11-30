# Copyright 2015-2023 Piperift - All rights reserved

add_executable(RiftEditor Src/main.cpp)

target_include_directories(RiftEditor PUBLIC Include)
file(GLOB_RECURSE RiftEditor_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftEditor PRIVATE ${RiftEditor_SOURCE_FILES})

target_link_libraries(RiftEditor PUBLIC
    RiftCompilerModules
    RiftEditorLib
    RiftEditorModules
)
target_compile_definitions(RiftEditor PUBLIC RIFT_EDITOR=1)

rift_module(RiftEditor)
set_icon(RiftEditor Icon.ico)
