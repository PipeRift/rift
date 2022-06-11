

add_library(RiftUI STATIC)
add_library(rift::UI ALIAS RiftUI)
target_include_directories(RiftUI PUBLIC Include)
file(GLOB_RECURSE UI_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftUI PRIVATE ${UI_SOURCE_FILES})
pipe_target_enable_CPP20(RiftUI)
set_target_properties (RiftUI PROPERTIES FOLDER Rift)
pipe_target_define_platform(RiftUI)
pipe_target_shared_output_directory(RiftUI)
target_compile_definitions(RiftUI PRIVATE NOMINMAX)
target_link_libraries(RiftUI PUBLIC
    Pipe
    glfw
    gl3w
    imgui
    IconFontCppHeaders
)

install(TARGETS RiftUI
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
