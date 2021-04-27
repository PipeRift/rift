

add_library(RiftECS STATIC)
add_library(Rift::ECS ALIAS RiftECS)
target_include_directories(RiftECS PUBLIC Include)
file(GLOB_RECURSE ECS_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftECS PRIVATE ${ECS_SOURCE_FILES})
rift_target_enable_CPP20(RiftECS)
set_target_properties (RiftECS PROPERTIES FOLDER Rift)
rift_target_define_platform(RiftECS)
rift_target_shared_output_directory(RiftECS)

target_link_libraries(RiftECS PUBLIC Rift::Core EnTT)

install(TARGETS RiftECS
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
