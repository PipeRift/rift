
include(FetchContent)

set(BUILD_SHARED_LIBS OFF)

if(RIFT_ENABLE_PROFILER)
    set(ENABLE_STATISTICS On)
endif()


# Submodule libraries
option(CLI11_BUILD_TESTS "Build CLI11 tests" OFF)
set(CMAKE_POLICY_DEFAULT_CMP0054 NEW)
add_subdirectory(CLI11)
add_subdirectory(date)
add_subdirectory(fmt)
target_compile_definitions(fmt PUBLIC TRACY_ENABLE=1)
add_subdirectory(glm)
add_subdirectory(SparseMap)
add_subdirectory(RobinMap)
add_subdirectory(Taskflow)
option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" ON)
add_subdirectory(spdlog)
option(JSON_BuildTests "Build the unit tests when BUILD_TESTING is enabled." OFF)
add_subdirectory(nlohmann_json)
add_subdirectory(robin_hood)
set(MI_OVERRIDE OFF CACHE BOOL "Override the standard malloc interface (e.g. define entry points for malloc() etc)")
set(MI_BUILD_TESTS OFF CACHE BOOL "Build test executables")
add_subdirectory(mimalloc)


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


# TRACY
add_library(Tracy STATIC)
target_include_directories(Tracy PUBLIC Tracy)
file(GLOB_RECURSE TRACY_SOURCES CONFIGURE_DEPENDS Tracy/TracyClient.cpp)
target_sources(Tracy PRIVATE ${TRACY_SOURCES})

set(TRACY_ENABLE ${RIFT_ENABLE_PROFILER})
target_compile_definitions(Tracy PUBLIC TRACY_ENABLE)
target_compile_definitions(Tracy PRIVATE _CRT_SECURE_NO_WARNINGS)
if(LINUX)
    find_library(DLPATH dl)
    target_link_libraries(Tracy PRIVATE ${DLPATH})
    # target_compile_definitions(Tracy PRIVATE TRACY_TIMER_QPC=1)
endif()

# Bandit (Tests)
add_library(Bandit INTERFACE)
target_include_directories(Bandit INTERFACE Bandit)
