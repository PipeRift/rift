
add_library(RiftEditor STATIC)
add_library(rift::Editor ALIAS RiftEditor)
target_include_directories(RiftEditor PUBLIC Include)
file(GLOB_RECURSE RiftEditor_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftEditor PRIVATE ${RiftEditor_SOURCE_FILES})
pipe_target_enable_CPP20(RiftEditor)
set_target_properties (RiftEditor PROPERTIES FOLDER Rift)
target_link_libraries(RiftEditor PUBLIC
    rift::Rift
    rift::Backend::LLVM
    rift::Backend::Cpp
    rift::UI
    CLI11
)
pipe_target_define_platform(RiftEditor)
pipe_target_shared_output_directory(RiftEditor)

set_icon(RiftEditor ../../Resources/Icon.ico)

install(TARGETS RiftEditor
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)