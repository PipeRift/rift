
add_executable(RiftCLIExe Src/main.cpp)
add_executable(rift::CLIExe ALIAS RiftCLIExe)

target_include_directories(RiftCLIExe PUBLIC Include)
file(GLOB_RECURSE CLI_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftCLIExe PRIVATE ${CLI_SOURCE_FILES})

target_link_libraries(RiftCLIExe PUBLIC
    rift::Rift
    rift::Backend::LLVM
    rift::View::Graph
    CLI11
)

rift_module(RiftCLIExe)
set_target_properties (RiftCLIExe PROPERTIES FOLDER Rift/Executables)
set_target_properties(RiftCLIExe PROPERTIES OUTPUT_NAME "Rift")
set_icon(RiftCLIExe ../../Resources/Icon.ico)

install(TARGETS RiftCLIExe
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)