
add_library(RiftBindingNative STATIC)


target_link_libraries(RiftBindingNative PUBLIC Rift)
target_link_libraries(RiftBindingNative PRIVATE RiftClang)

rift_compiler_module(RiftBindingNative)

install(TARGETS RiftBindingNative
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)

add_library(RiftBindingNativeEditor STATIC)
rift_editor_module(RiftBindingNativeEditor)
target_link_libraries(RiftBindingNativeEditor PUBLIC RiftEditor RiftBindingNative)
