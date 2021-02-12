
add_executable(Editor Src/main.cpp)
set_target_properties(Editor PROPERTIES OUTPUT_NAME "RiftEditor")
target_include_directories(Editor PUBLIC Include)
file(GLOB_RECURSE EDITOR_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(Editor PRIVATE ${EDITOR_SOURCE_FILES})
set_target_properties (Editor PROPERTIES
    CXX_STANDARD 20
    FOLDER Rift
)
target_link_libraries(Editor PUBLIC ${PROJECT_NAME} sokol imgui)
rift_target_define_platform(Editor)
rift_target_shared_output_directory(Editor)

install(TARGETS Editor
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)