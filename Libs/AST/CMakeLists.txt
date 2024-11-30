# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftASTLib STATIC)
target_include_directories(RiftASTLib PUBLIC Include)

file(GLOB_RECURSE AST_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftASTLib PRIVATE ${AST_SOURCE_FILES})

target_link_libraries(RiftASTLib PUBLIC
    Pipe
    Taskflow
    RiftBindingNativeLib
)

rift_module(RiftASTLib)
set_target_properties (RiftASTLib PROPERTIES FOLDER Rift)
