
file(GLOB_RECURSE BACKEND_LLVM_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(RiftBackendLLVM STATIC)
add_library(rift::Backend::LLVM ALIAS RiftBackendLLVM)
target_include_directories(RiftBackendLLVM PUBLIC Include)
target_sources(RiftBackendLLVM PRIVATE ${BACKEND_LLVM_SOURCE_FILES})
pipe_target_enable_CPP20(RiftBackendLLVM)
set_target_properties (RiftBackendLLVM PROPERTIES FOLDER Rift)
pipe_target_define_platform(RiftBackendLLVM)
pipe_target_shared_output_directory(RiftBackendLLVM)

target_link_libraries(RiftBackendLLVM PUBLIC rift::Rift)

target_link_libraries(RiftBackendLLVM PRIVATE RiftLLVM)


install(TARGETS RiftBackendLLVM
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
