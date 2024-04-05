# Fetch the Google Test framework
cmake_minimum_required(VERSION 3.22)
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
  LANGUAGES C ASM
  VERSION 0.1.0
  DESCRIPTION "Raptor unittest binary."
)

enable_testing()

add_executable(
  ${PROJECT_NAME}
 test_sysreg.cc
)

include(../../lib)

target_link_libraries(
  ${PROJECT_NAME}
  GTest::gtest_main
  ${RAPTOR_LIB}
)

include(GoogleTest)
include(CodeCoverage)

setup_target_for_coverage_gcovr_xml(
  NAME coverage
  EXECUTABLE 
)

gtest_discover_tests(${PROJECT_NAME})

