
add_library(RiftEditor STATIC)

target_include_directories(RiftEditor PUBLIC Include)
file(GLOB_RECURSE RiftEditor_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftEditor PRIVATE ${RiftEditor_SOURCE_FILES})

target_link_libraries(RiftEditor PUBLIC
    Rift
    RiftBackendLLVM
    RiftUI
    CLI11
)

rift_module(RiftEditor)
rift_enable_module_resources(RiftEditor)
set_icon(RiftEditor ../../Resources/Icon.ico)

install(TARGETS RiftEditor
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)