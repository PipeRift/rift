
add_executable(RiftEditorExe Src/main.cpp)
add_executable(rift::EditorExe ALIAS RiftEditorExe)

target_include_directories(RiftEditorExe PUBLIC Include)
file(GLOB_RECURSE RiftEditor_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftEditorExe PRIVATE ${RiftEditor_SOURCE_FILES})

target_link_libraries(RiftEditorExe PUBLIC
    rift::Editor
    rift::Backend::LLVM
    rift::View::Graph
)

rift_module(RiftEditorExe)
set_target_properties (RiftEditorExe PROPERTIES FOLDER Rift/Executables)
set_target_properties(RiftEditorExe PROPERTIES OUTPUT_NAME "RiftEditor")
set_icon(RiftEditorExe Icon.ico)

install(TARGETS RiftEditorExe
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
