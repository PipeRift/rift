
include(FetchContent)

set(BUILD_SHARED_LIBS OFF)

# Submodule libraries
set(CLI11_BUILD_TESTS OFF CACHE BOOL "Build CLI11 tests")
set(CMAKE_POLICY_DEFAULT_CMP0054 NEW)
add_subdirectory(CLI11)


find_package(OpenGL REQUIRED)
add_library(gl3w STATIC gl3w/GL/gl3w.c)
target_include_directories(gl3w PUBLIC gl3w)
target_link_libraries(gl3w INTERFACE OpenGL::GL OpenGL::GLU)

add_subdirectory(glfw)

# Imgui
add_library(imgui STATIC)
target_include_directories(imgui PUBLIC imgui)
target_sources(imgui PRIVATE
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
    imgui/misc/fonts/binary_to_compressed_c.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
rift_target_enable_CPP20(imgui)
set_target_properties (imgui PROPERTIES FOLDER Extern)
target_link_libraries(imgui PRIVATE glfw gl3w)

add_library(imnodes STATIC)
target_include_directories(imnodes PUBLIC imnodes)
target_sources(imnodes PRIVATE imnodes/imnodes.cpp)
target_link_libraries(imnodes PRIVATE imgui)
target_compile_definitions(imnodes PRIVATE _CRT_SECURE_NO_WARNINGS)

