
file(GLOB_RECURSE FRAMEWORK_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(Rift STATIC)
add_library(rift::Rift ALIAS Rift)
target_include_directories(Rift PUBLIC Include)
target_sources(Rift PRIVATE ${FRAMEWORK_SOURCE_FILES})
rift_target_enable_CPP20(Rift)
set_target_properties (Rift PROPERTIES FOLDER Rift)
rift_target_define_platform(Rift)
rift_target_shared_output_directory(Rift)
target_link_libraries(Rift PUBLIC
    Pipe
)

install(TARGETS Rift
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
