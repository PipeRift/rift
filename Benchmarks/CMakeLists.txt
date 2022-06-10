
file(GLOB_RECURSE BENCHMARKS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(RiftBenchmarks ${BENCHMARKS_SOURCE_FILES})
add_executable(rift::Benchmarks ALIAS RiftBenchmarks)
target_include_directories(RiftBenchmarks PUBLIC .)
pipe_target_enable_CPP20(RiftBenchmarks)
pipe_target_define_platform(RiftBenchmarks)
pipe_target_shared_output_directory(RiftBenchmarks)
target_link_libraries(RiftBenchmarks PUBLIC rift::Rift)
