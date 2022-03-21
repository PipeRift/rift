
file(GLOB_RECURSE VIEW_NODES_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(RiftViewGraph STATIC)
add_library(Rift::View::Graph ALIAS RiftViewGraph)
target_include_directories(RiftViewGraph PUBLIC Include)
target_sources(RiftViewGraph PRIVATE ${VIEW_NODES_SOURCE_FILES})
rift_target_enable_CPP20(RiftViewGraph)
set_target_properties (RiftViewGraph PROPERTIES FOLDER Rift)
rift_target_define_platform(RiftViewGraph)
rift_target_shared_output_directory(RiftViewGraph)
target_link_libraries(RiftViewGraph PUBLIC
    Rift::Rift
)

install(TARGETS RiftViewGraph
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
