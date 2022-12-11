
add_executable(RiftCLIExe Src/main.cpp)

target_include_directories(RiftCLIExe PUBLIC Include)
file(GLOB_RECURSE CLI_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftCLIExe PRIVATE ${CLI_SOURCE_FILES})

target_link_libraries(RiftCLIExe PUBLIC
    CLI11
    Rift
    ${RIFT_RUNTIME_MODULES}
)

rift_module(RiftCLIExe)
set_target_properties(RiftCLIExe PROPERTIES OUTPUT_NAME "Rift")
set_icon(RiftCLIExe ../../Resources/Icon.ico)

install(TARGETS RiftCLIExe
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)