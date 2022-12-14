
add_library(RiftViewGraph STATIC)
rift_runtime_module(RiftViewGraph)
target_link_libraries(RiftViewGraph PUBLIC Rift)

install(TARGETS RiftViewGraph
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)

add_library(RiftViewGraphEditor STATIC)
rift_editor_module(RiftViewGraphEditor)
target_link_libraries(RiftViewGraphEditor PUBLIC RiftEditor RiftViewGraph)
