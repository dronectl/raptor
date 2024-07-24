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
  LANGUAGES C CXX
  DESCRIPTION "Raptor unittest binary."
)

enable_testing()
set(LIB_RAPTOR "libraptor")
add_library(
  ${LIB_RAPTOR}
  ../../core/lib/sysreg.c
  ../../core/lib/scpi/utf8.c
)
target_include_directories(
  ${LIB_RAPTOR}
  PUBLIC
  ../../core/lib
  ../../core/lib/scpi
)
target_compile_options(${LIB_RAPTOR} PUBLIC -g -O0 -ftest-coverage -fprofile-arcs)
target_link_options(${LIB_RAPTOR} PUBLIC --coverage)

add_executable(
  ${PROJECT_NAME}
  test_sysreg.cc
  test_utf8.cc
)
target_compile_options(${PROJECT_NAME} PUBLIC -g -O0)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB_RAPTOR} GTest::gtest_main)

include(CTest)
include(GoogleTest)
include(CodeCoverage.cmake)

set(GCOVR_ADDITIONAL_ARGS --print-summary)
setup_target_for_coverage_gcovr_xml(
  NAME coverage
  EXECUTABLE ctest
  BASE_DIRECTORY "../../"
  DEPENDANCIES ${LIB_RAPTOR}
)


gtest_discover_tests(${PROJECT_NAME})

