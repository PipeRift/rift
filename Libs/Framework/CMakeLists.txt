
add_library(${PROJECT_NAME} SHARED Src/Framework.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC Include)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME} PUBLIC Core)


install(TARGETS ${PROJECT_NAME}
    EXPORT VCLangTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
