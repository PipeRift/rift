
add_library(RiftUI STATIC)

target_include_directories(RiftUI PUBLIC Include)
file(GLOB_RECURSE UI_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftUI PRIVATE ${UI_SOURCE_FILES})

target_link_libraries(RiftUI PUBLIC
    Pipe
    glfw
    gl3w
    imgui
    IconFontCppHeaders
)
target_compile_definitions(RiftUI PRIVATE NOMINMAX)

rift_module(RiftUI)

install(TARGETS RiftUI
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
