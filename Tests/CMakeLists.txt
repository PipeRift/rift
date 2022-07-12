
file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(RiftTests ${TESTS_SOURCE_FILES})
add_executable(rift::Tests ALIAS RiftTests)
target_include_directories(RiftTests PUBLIC .)
pipe_target_enable_CPP20(RiftTests)
set_target_properties (RiftTests PROPERTIES FOLDER Rift)
pipe_target_define_platform(RiftTests)
pipe_target_shared_output_directory(RiftTests)
target_link_libraries(RiftTests PUBLIC ${PROJECT_NAME} Bandit)

if(PIPE_ENABLE_PROFILER)
    # Force profiler disabled in tests
    target_compile_definitions(RiftTests PUBLIC PIPE_ENABLE_PROFILER=0)
endif()
add_test(NAME RiftTests COMMAND $<TARGET_FILE:RiftTests>)
