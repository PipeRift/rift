
file(GLOB_RECURSE BACKEND_LLVM_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(RiftBackendLLVM SHARED)
add_library(Rift::Backend::LLVM ALIAS RiftBackendLLVM)
target_include_directories(RiftBackendLLVM PUBLIC Include)
target_sources(RiftBackendLLVM PRIVATE ${BACKEND_LLVM_SOURCE_FILES})
rift_target_enable_CPP20(RiftBackendLLVM)
set_target_properties (RiftBackendLLVM PROPERTIES FOLDER Rift)
rift_target_define_platform(RiftBackendLLVM)
rift_target_shared_output_directory(RiftBackendLLVM)
target_link_libraries(RiftBackendLLVM PUBLIC
    Rift::Rift
    ${llvm_libs}
)

install(TARGETS RiftBackendLLVM
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
