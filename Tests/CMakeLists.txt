
file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(Tests ${TESTS_SOURCE_FILES})
target_include_directories(Tests PUBLIC .)
set_property(TARGET Tests PROPERTY CXX_STANDARD 17)
target_link_libraries(Tests PUBLIC ${PROJECT_NAME} Bandit)
target_define_platform(Tests)

target_shared_output_directory(Tests)

