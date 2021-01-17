
add_executable(Editor Src/main.cpp)
set_target_properties(Editor PROPERTIES OUTPUT_NAME "Rift_Editor")
target_include_directories(Editor PUBLIC Include)
set_property(TARGET Editor PROPERTY CXX_STANDARD 17)
target_link_libraries(Editor PUBLIC ${PROJECT_NAME} imgui)
target_define_platform(Editor)

target_shared_output_directory(Editor)

install(TARGETS Editor
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)