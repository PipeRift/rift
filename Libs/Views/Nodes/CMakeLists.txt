
file(GLOB_RECURSE VIEW_NODES_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(RiftViewNodes STATIC)
add_library(Rift::View::Nodes ALIAS RiftViewNodes)
target_include_directories(RiftViewNodes PUBLIC Include)
target_sources(RiftViewNodes PRIVATE ${VIEW_NODES_SOURCE_FILES})
rift_target_enable_CPP20(RiftViewNodes)
set_target_properties (RiftViewNodes PROPERTIES FOLDER Rift)
rift_target_define_platform(RiftViewNodes)
rift_target_shared_output_directory(RiftViewNodes)
target_link_libraries(RiftViewNodes PUBLIC
    Rift::Rift
)

install(TARGETS RiftViewNodes
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
