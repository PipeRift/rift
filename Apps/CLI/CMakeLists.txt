
add_executable(RiftCLIExe Src/main.cpp)
add_executable(Rift::CLIExe ALIAS RiftCLIExe)
set_target_properties(RiftCLIExe PROPERTIES OUTPUT_NAME "Rift")

target_include_directories(RiftCLIExe PUBLIC Include)
file(GLOB_RECURSE CLI_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftCLIExe PRIVATE ${CLI_SOURCE_FILES})
rift_target_enable_CPP20(RiftCLIExe)
set_target_properties (RiftCLIExe PROPERTIES FOLDER Rift/Executables)
target_link_libraries(RiftCLIExe PUBLIC
    Rift::Rift
    Rift::Backend::LLVM
    Rift::Backend::Cpp
    Rift::View::Graph
    CLI11
)
rift_target_define_platform(RiftCLIExe)
rift_target_shared_output_directory(RiftCLIExe)

set_icon(RiftCLIExe ../../Resources/Icon.ico)

install(TARGETS RiftCLIExe
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)