
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

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
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
pipe_target_enable_CPP20(imgui)
set_target_properties (imgui PROPERTIES FOLDER Extern)
target_link_libraries(imgui PRIVATE glfw gl3w)

add_library(IconFontCppHeaders INTERFACE)
target_include_directories(IconFontCppHeaders INTERFACE IconFontCppHeaders)


set(TF_BUILD_TESTS OFF CACHE BOOL "Enables builds of tests")
set(TF_BUILD_EXAMPLES OFF CACHE BOOL "Enables builds of examples")
add_subdirectory(taskflow)
pipe_target_disable_all_warnings(Taskflow INTERFACE)


add_subdirectory(mir)
target_include_directories(mir_static PUBLIC mir)

