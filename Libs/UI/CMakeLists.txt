

add_library(RiftUI STATIC)
add_library(Rift::UI ALIAS RiftUI)
target_include_directories(RiftUI PUBLIC Include)
file(GLOB_RECURSE UI_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftUI PRIVATE ${UI_SOURCE_FILES})
rift_target_enable_CPP20(RiftUI)
set_target_properties (RiftUI PROPERTIES FOLDER Rift)
rift_target_define_platform(RiftUI)
rift_target_shared_output_directory(RiftUI)
target_compile_definitions(RiftUI PRIVATE NOMINMAX)
target_link_libraries(RiftUI PUBLIC
    Rift::Core
    glfw
    gl3w
    imgui
    imnodes
)

install(TARGETS RiftUI
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
