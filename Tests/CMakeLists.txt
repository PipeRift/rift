
file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(RiftTests ${TESTS_SOURCE_FILES})
add_executable(Rift::Tests ALIAS RiftTests)
target_include_directories(RiftTests PUBLIC .)
rift_target_enable_CPP20(RiftTests)
set_target_properties (RiftTests PROPERTIES FOLDER Rift)
rift_target_define_platform(RiftTests)
rift_target_shared_output_directory(RiftTests)
target_link_libraries(RiftTests PUBLIC ${PROJECT_NAME} Bandit)

add_test(NAME RiftTests COMMAND $<TARGET_FILE:RiftTests>)
