
add_library(RiftViewGraph STATIC)
add_library(rift::View::Graph ALIAS RiftViewGraph)

target_include_directories(RiftViewGraph PUBLIC Include)
file(GLOB_RECURSE VIEW_NODES_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftViewGraph PRIVATE ${VIEW_NODES_SOURCE_FILES})

target_link_libraries(RiftViewGraph PUBLIC rift::Rift)

rift_module(RiftViewGraph)
set_target_properties (RiftViewGraph PROPERTIES FOLDER Rift)

install(TARGETS RiftViewGraph
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
