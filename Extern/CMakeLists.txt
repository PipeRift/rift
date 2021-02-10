
include(FetchContent)

set(BUILD_SHARED_LIBS OFF)

if(RIFT_ENABLE_PROFILER)
    set(ENABLE_STATISTICS On)
endif()


# Submodule libraries
set(CLI11_BUILD_TESTS OFF CACHE BOOL "Build CLI11 tests")
set(CMAKE_POLICY_DEFAULT_CMP0054 NEW)
add_subdirectory(CLI11)


# Imgui
add_library(imgui STATIC)
target_include_directories(imgui PUBLIC imgui)
target_sources(imgui PRIVATE
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
    imgui/misc/fonts/binary_to_compressed_c.cpp
)
set_property(TARGET imgui PROPERTY CXX_STANDARD 17)
