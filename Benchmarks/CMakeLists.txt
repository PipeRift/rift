
file(GLOB_RECURSE BENCHMARKS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(RiftBenchmarks ${BENCHMARKS_SOURCE_FILES})
add_executable(Rift::Benchmarks ALIAS RiftBenchmarks)
target_include_directories(RiftBenchmarks PUBLIC .)
rift_target_enable_CPP20(RiftBenchmarks)
rift_target_define_platform(RiftBenchmarks)
rift_target_shared_output_directory(RiftBenchmarks)
target_link_libraries(RiftBenchmarks PUBLIC Rift::Rift)
