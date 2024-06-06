# Fetch the Google Test framework
cmake_minimum_required(VERSION 3.22)

include(FetchContent)
message(STATUS "Fetching googletest ...")
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG v1.14.0
  GIT_PROGRESS TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

message(STATUS "Fetching fake function framework ...")
# HACK: find a way to use root cmake to build
enable_testing()
add_executable(test_raptor ../../core/lib/sysreg.c test_sysreg.cc)
target_include_directories(test_raptor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests ../../core/lib)
target_link_libraries(test_raptor PRIVATE GTest::gtest_main)
target_compile_options(test_raptor PUBLIC -stdlib=libc++)

include(GoogleTest)

# include(CodeCoverage.cmake)
# 
# setup_target_for_coverage_gcovr_html(
#   NAME coverage
#   EXECUTABLE GTEST_COLOR=1 ctest -V
#   --object-directory ${CMAKE_BINARY_DIR}/lib/CMakeFiles/libraptor.dir/src
# )

gtest_discover_tests(test_raptor)

