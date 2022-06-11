
add_executable(RiftEditorExe Src/main.cpp)
add_executable(rift::EditorExe ALIAS RiftEditorExe)
set_target_properties(RiftEditorExe PROPERTIES OUTPUT_NAME "RiftEditor")

target_include_directories(RiftEditorExe PUBLIC Include)
file(GLOB_RECURSE RiftEditor_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftEditorExe PRIVATE ${RiftEditor_SOURCE_FILES})
pipe_target_enable_CPP20(RiftEditorExe)
set_target_properties (RiftEditorExe PROPERTIES FOLDER Rift/Executables)
target_link_libraries(RiftEditorExe PUBLIC
    rift::Editor
    rift::Backend::LLVM
    rift::Backend::Cpp
    rift::View::Graph
)
pipe_target_define_platform(RiftEditorExe)
pipe_target_shared_output_directory(RiftEditorExe)

set_icon(RiftEditorExe ../../Resources/Icon.ico)

install(TARGETS RiftEditorExe
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
