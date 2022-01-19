
add_executable(RiftCLI Src/main.cpp)
add_executable(Rift::CLI ALIAS RiftCLI)
set_target_properties(RiftCLI PROPERTIES OUTPUT_NAME "Rift")
target_include_directories(RiftCLI PUBLIC Include)
file(GLOB_RECURSE CLI_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftCLI PRIVATE ${CLI_SOURCE_FILES})
rift_target_enable_CPP20(RiftCLI)
set_target_properties (RiftCLI PROPERTIES FOLDER Rift)
target_link_libraries(RiftCLI PUBLIC Rift::Rift CLI11)
rift_target_define_platform(RiftCLI)
rift_target_shared_output_directory(RiftCLI)

set_icon(RiftCLI ../../Resources/Icon.ico)

install(TARGETS RiftCLI
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)