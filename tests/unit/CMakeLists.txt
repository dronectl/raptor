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
  LANGUAGES C ASM
  DESCRIPTION "Raptor unittest binary."
)

enable_testing()

add_executable(
  ${PROJECT_NAME}
 test_sysreg.cc
)

set(RAPTOR_LIB libraptor)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../lib build)

target_link_libraries(
  ${PROJECT_NAME}
  GTest::gtest_main
  ${RAPTOR_LIB}
)

include(GoogleTest)

gtest_discover_tests(${PROJECT_NAME})

