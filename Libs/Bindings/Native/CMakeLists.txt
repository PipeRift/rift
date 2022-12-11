
add_library(RiftBindingNative STATIC)
add_library(rift::Bindings::Native ALIAS RiftBindingNative)

target_include_directories(RiftBindingNative PUBLIC Include)
file(GLOB_RECURSE BACKEND_CPP_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftBindingNative PRIVATE ${BACKEND_CPP_SOURCE_FILES})

target_link_libraries(RiftBindingNative PUBLIC rift::Rift)
target_link_libraries(RiftBindingNative PRIVATE RiftClang)

rift_module(RiftBindingNative)
set_target_properties (RiftBindingNative PROPERTIES FOLDER Rift)

install(TARGETS RiftBindingNative
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
