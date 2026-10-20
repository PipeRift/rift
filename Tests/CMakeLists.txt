# Copyright 2015-2023 Piperift - All rights reserved

file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)
add_executable(RiftTests ${TESTS_SOURCE_FILES})
rift_module(RiftTests)
pipe_target_shared_output_directory(RiftTests)
target_include_directories(RiftTests PUBLIC .)
target_link_libraries(RiftTests PUBLIC RiftASTLib Pipe)

# The standard runtime is compiled into the tests to check its operations
file(GLOB_RECURSE STD_SOURCE_FILES CONFIGURE_DEPENDS ../Libs/Runtimes/Std/Src/*.cpp)
target_sources(RiftTests PRIVATE ${STD_SOURCE_FILES})
target_include_directories(RiftTests PUBLIC ../Libs/Runtimes/Std/Include)

add_test(NAME RiftTests COMMAND $<TARGET_FILE:RiftTests>)

set_icon(RiftTests Icon.ico)
