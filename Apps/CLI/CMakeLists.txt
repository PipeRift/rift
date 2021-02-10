
add_executable(CLI Src/main.cpp)
set_target_properties(CLI PROPERTIES OUTPUT_NAME "Rift")
target_include_directories(CLI PUBLIC Include)
set_property(TARGET CLI PROPERTY CXX_STANDARD 20)
target_link_libraries(CLI PUBLIC ${PROJECT_NAME} CLI11)
rift_target_define_platform(CLI)
rift_target_shared_output_directory(CLI)

install(TARGETS CLI
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)