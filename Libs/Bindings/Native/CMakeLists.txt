
file(GLOB_RECURSE BACKEND_CPP_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(RiftBindingNative STATIC)
add_library(rift::Bindings::Native ALIAS RiftBindingNative)
target_include_directories(RiftBindingNative PUBLIC Include)
target_sources(RiftBindingNative PRIVATE ${BACKEND_CPP_SOURCE_FILES})
pipe_target_enable_CPP20(RiftBindingNative)
set_target_properties (RiftBindingNative PROPERTIES FOLDER Rift)
pipe_target_define_platform(RiftBindingNative)
pipe_target_shared_output_directory(RiftBindingNative)

target_link_libraries(RiftBindingNative PUBLIC rift::Rift)
target_link_libraries(RiftBindingNative PRIVATE RiftClang)

install(TARGETS RiftBindingNative
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
