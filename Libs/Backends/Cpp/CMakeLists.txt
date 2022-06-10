
file(GLOB_RECURSE BACKEND_CPP_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(RiftBackendCpp STATIC)
add_library(rift::Backend::Cpp ALIAS RiftBackendCpp)
target_include_directories(RiftBackendCpp PUBLIC Include)
target_sources(RiftBackendCpp PRIVATE ${BACKEND_CPP_SOURCE_FILES})
pipe_target_enable_CPP20(RiftBackendCpp)
set_target_properties (RiftBackendCpp PROPERTIES FOLDER Rift)
pipe_target_define_platform(RiftBackendCpp)
pipe_target_shared_output_directory(RiftBackendCpp)
target_link_libraries(RiftBackendCpp PUBLIC
    rift::Rift
)

install(TARGETS RiftBackendCpp
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
