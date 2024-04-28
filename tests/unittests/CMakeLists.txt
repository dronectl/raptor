# Fetch the Google Test framework
cmake_minimum_required(VERSION 3.22)

# specify module path
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)
message(STATUS "Fetching Google Test...")
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG v1.14.0
  GIT_PROGRESS TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(
  raptor-test
  LANGUAGES C CXX
  DESCRIPTION "Lib raptor unittest binary."
)
enable_testing()

set(RAPTOR_LIB "libraptor")
add_subdirectory(${CMAKE_SOURCE_DIR}/../../lib ${CMAKE_BINARY_DIR}/lib)
add_executable(${PROJECT_NAME} test_sysreg.cc)
target_compile_options(${RAPTOR_LIB} PUBLIC -g -O0 --coverage -fprofile-arcs -ftest-coverage)
target_link_options(${RAPTOR_LIB} PUBLIC --coverage)
target_compile_options(${PROJECT_NAME} PUBLIC -g -O0)
target_link_libraries(${PROJECT_NAME} ${RAPTOR_LIB} GTest::gtest_main)

include(GoogleTest)
include(CodeCoverage.cmake)

setup_target_for_coverage_gcovr_html(
  NAME coverage
  EXECUTABLE GTEST_COLOR=1 ctest -V
  --object-directory ${CMAKE_BINARY_DIR}/lib/CMakeFiles/libraptor.dir/src
)

gtest_discover_tests(${PROJECT_NAME})

