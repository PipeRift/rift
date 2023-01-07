# Copyright 2015-2023 Piperift - All rights reserved

add_library(Rift STATIC)
target_include_directories(Rift PUBLIC Include)

file(GLOB_RECURSE FRAMEWORK_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(Rift PRIVATE ${FRAMEWORK_SOURCE_FILES})

target_link_libraries(Rift PUBLIC
    Pipe
    Taskflow
    RiftBindingNative
)

rift_module(Rift)
set_target_properties (Rift PROPERTIES FOLDER Rift)

install(TARGETS Rift
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
