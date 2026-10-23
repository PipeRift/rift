# Copyright 2015-2023 Piperift - All rights reserved

add_executable(RiftCLI Src/main.cpp)

target_include_directories(RiftCLI PUBLIC Include)
file(GLOB_RECURSE CLI_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftCLI PRIVATE ${CLI_SOURCE_FILES})

target_link_libraries(RiftCLI PUBLIC
    CLI11
    RiftASTLib
    RiftCompilerModules
)

rift_module(RiftCLI)
set_target_properties(RiftCLI PROPERTIES OUTPUT_NAME "Rift")
set_icon(RiftCLI Icon.ico)
