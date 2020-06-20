
#CONAN PACKAGES
conan_cmake_run(REQUIRES eastl/3.16.05 BASIC_SETUP CMAKE_TARGETS BUILD missing)
conan_cmake_run(REQUIRES glm/0.9.9.8 BASIC_SETUP CMAKE_TARGETS BUILD missing)

# Taskflow
add_library(Taskflow INTERFACE)
target_include_directories(GLM INTERFACE GLM/glm)


# TRACY
add_library(Tracy STATIC)
target_include_directories(Tracy PUBLIC Tracy)
target_sources(Tracy PRIVATE Tracy/TracyClient.cpp)
target_compile_features(Tracy PUBLIC cxx_std_17)

if(WINDOWS)
    target_compile_definitions(Tracy PRIVATE _WIN32=1)
elseif(LINUX)
    target_compile_definitions(Tracy PRIVATE __linux__=1)
endif()

if(ENABLE_PROFILER)
    target_compile_definitions(Tracy PUBLIC TRACY_ENABLE=1)
endif()
