
add_executable(Editor Src/main.cpp)
set_target_properties(Editor PROPERTIES OUTPUT_NAME "VCLang_Editor")
target_include_directories(Editor PUBLIC Include)
target_compile_features(Editor PUBLIC cxx_std_17)
target_link_libraries(Editor PUBLIC ${PROJECT_NAME} CONAN_PKG::imgui)
target_define_platform(Editor)

target_shared_output_directory(Editor)

install(TARGETS Editor
    EXPORT VCLangTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)