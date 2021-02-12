
file(GLOB_RECURSE FRAMEWORK_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(${PROJECT_NAME} STATIC)
target_include_directories(${PROJECT_NAME} PUBLIC Include)
target_sources(${PROJECT_NAME} PRIVATE ${FRAMEWORK_SOURCE_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_target_properties (${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    FOLDER Rift
)
rift_target_define_platform(${PROJECT_NAME})
rift_target_shared_output_directory(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PUBLIC rift::core)

install(TARGETS ${PROJECT_NAME}
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
