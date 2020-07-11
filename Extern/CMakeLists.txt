
include(FetchContent)

# Submodule libraries
option(CLI11_BUILD_TESTS "Build CLI11 tests" OFF)
add_subdirectory(CLI11)
add_subdirectory(date)
add_subdirectory(EABase)
add_subdirectory(fmt)
add_subdirectory(glm)
add_subdirectory(SparseMap)
add_subdirectory(RobinMap)
add_subdirectory(Taskflow)
option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" ON)
add_subdirectory(spdlog)
option(JSON_BuildTests "Build the unit tests when BUILD_TESTING is enabled." OFF)
add_subdirectory(nlohmann_json)

# TRACY
add_library(Tracy STATIC)
target_include_directories(Tracy PUBLIC Tracy)
target_sources(Tracy PRIVATE Tracy/TracyClient.cpp)
set_property(TARGET Tracy PROPERTY CXX_STANDARD 17)
if(LINUX)
    find_library(DLPATH dl)
    target_link_libraries(Tracy PRIVATE ${DLPATH})
endif()

# EASTL
file(GLOB_RECURSE EASTL_SOURCES CONFIGURE_DEPENDS EASTL/source/*.cpp)
add_library(EASTL STATIC)
target_include_directories(EASTL PUBLIC EASTL/include)
target_sources(EASTL PRIVATE ${EASTL_SOURCES})
set_property(TARGET EASTL PROPERTY CXX_STANDARD 17)
target_link_libraries(EASTL EABase)

# Imgui
file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS imgui/misc/*.cpp)
add_library(imgui STATIC)
target_include_directories(imgui PUBLIC imgui)
target_sources(imgui PRIVATE
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    ${IMGUI_SOURCES}
)
set_property(TARGET imgui PROPERTY CXX_STANDARD 17)


if(WINDOWS)
    target_compile_definitions(Tracy PRIVATE _WIN32=1)
elseif(LINUX)
    target_compile_definitions(Tracy PRIVATE __linux__=1)
endif()

if(VCLANG_ENABLE_PROFILER)
    target_compile_definitions(Tracy PUBLIC TRACY_ENABLE=1)
endif()
