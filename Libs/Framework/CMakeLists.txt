
add_library(Rift STATIC)
add_library(rift::Rift ALIAS Rift)
target_include_directories(Rift PUBLIC Include)

file(GLOB_RECURSE FRAMEWORK_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(Rift PRIVATE ${FRAMEWORK_SOURCE_FILES})

target_link_libraries(Rift PUBLIC
    Pipe
    Taskflow
    rift::Bindings::Native
)

rift_module(Rift)
set_target_properties (Rift PROPERTIES FOLDER Rift)

install(TARGETS Rift
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
