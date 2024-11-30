# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftUILib STATIC)

target_include_directories(RiftUILib PUBLIC Include)
file(GLOB_RECURSE UI_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftUILib PRIVATE ${UI_SOURCE_FILES})

target_link_libraries(RiftUILib PUBLIC
    Pipe
    glfw
    gl3w
    imgui
    IconFontCppHeaders
)
target_link_libraries(RiftUILib PRIVATE stb_image)
target_compile_definitions(RiftUILib PRIVATE NOMINMAX)

rift_module(RiftUILib)
