
add_library(${PROJECT_NAME} STATIC Src/Framework.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC Include)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
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
