
add_library(${PROJECT_NAME} STATIC)

target_include_directories(${PROJECT_NAME} PUBLIC Include)

file(GLOB_RECURSE FRAMEWORK_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(${PROJECT_NAME} PRIVATE ${FRAMEWORK_SOURCE_FILES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PUBLIC Core)
target_define_platform(${PROJECT_NAME})

target_shared_output_directory(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    EXPORT VCLangTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
