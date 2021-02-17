
add_executable(RiftEditor Src/main.cpp)
add_executable(Rift::Editor ALIAS RiftEditor)
target_include_directories(RiftEditor PUBLIC Include)
file(GLOB_RECURSE RiftEditor_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftEditor PRIVATE ${RiftEditor_SOURCE_FILES})
rift_target_enable_CPP20(RiftEditor)
set_target_properties (RiftEditor PROPERTIES FOLDER Rift)
target_link_libraries(RiftEditor PUBLIC Rift::Rift sokol imgui)
rift_target_define_platform(RiftEditor)
rift_target_shared_output_directory(RiftEditor)

install(TARGETS RiftEditor
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)