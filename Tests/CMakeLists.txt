
file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(RiftTests ${TESTS_SOURCE_FILES})
add_executable(Rift::Tests ALIAS RiftTests)
target_include_directories(RiftTests PUBLIC .)
set_target_properties (RiftTests PROPERTIES
    CXX_STANDARD 20
    FOLDER Rift
)
rift_target_define_platform(RiftTests)
rift_target_shared_output_directory(RiftTests)
target_link_libraries(RiftTests PUBLIC ${PROJECT_NAME} Bandit)
