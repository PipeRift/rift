# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftAST STATIC)
target_include_directories(RiftAST PUBLIC Include)

file(GLOB_RECURSE AST_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftAST PRIVATE ${AST_SOURCE_FILES})

target_link_libraries(RiftAST PUBLIC
    Pipe
    Taskflow
    RiftBindingNative
)

rift_module(RiftAST)
set_target_properties (RiftAST PROPERTIES FOLDER Rift)
