cmake_minimum_required(VERSION 3.14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Fetch the Google Test framework
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
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "-O0 -g")
set(CMAKE_CXX_FLAGS "-O0 -g")

set(EXECUTABLE "raptor_test")
project(
  ${EXECUTABLE}
  LANGUAGES CXX C
  DESCRIPTION "Gtest powered unittest."
)

# relative path
set(RAPTOR_ROOT "../..")

enable_testing()

include_directories(${RAPTOR_ROOT}/include)

add_executable(
  ${EXECUTABLE}
  ${RAPTOR_ROOT}/src/sysreg.c
  test_sysreg.cc
)

target_link_libraries(
  ${EXECUTABLE}
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${EXECUTABLE})

