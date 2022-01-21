
file(GLOB_RECURSE BACKEND_CPP_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(RiftBackendCPP STATIC)
add_library(Rift::Backend::CPP ALIAS RiftBackendCPP)
target_include_directories(RiftBackendCPP PUBLIC Include)
target_sources(RiftBackendCPP PRIVATE ${BACKEND_CPP_SOURCE_FILES})
rift_target_enable_CPP20(RiftBackendCPP)
set_target_properties (RiftBackendCPP PROPERTIES FOLDER Rift)
rift_target_define_platform(RiftBackendCPP)
rift_target_shared_output_directory(RiftBackendCPP)
target_link_libraries(RiftBackendCPP PUBLIC
    Rift::Rift
    ${llvm_libs}
)

install(TARGETS RiftBackendCPP
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
