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
  DESCRIPTION "Raptor unittests"
)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(FATAL_ERROR "This project requires a GNU GCC compiler. The current compiler is ${CMAKE_CXX_COMPILER_ID}.")
endif()
if (NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
  message(FATAL_ERROR "This project requires a GNU GCC compiler. The current compiler is ${CMAKE_C_COMPILER_ID}.")
endif()

enable_testing()
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(LIB_RAPTOR "libraptor")
# TODO: include_subdirectories
add_library(
  ${LIB_RAPTOR}
  ../src/common/sysreg.c
  ../src/common/logger.c
  ../src/common/cbuffer.c
  ../src/os/scpi/utf8.c
  ../src/os/scpi/err.c
  ../src/os/scpi/commands.c
  ../src/os/scpi/lexer.c
  ../src/os/scpi/parser.c
)

set(LIB_MOCK "libmock")
add_subdirectory(mock)

target_include_directories(
  ${LIB_RAPTOR}
  PUBLIC
  ../src/common
  ../src/drivers
  ../src/os
)
target_compile_options(${LIB_RAPTOR} PUBLIC -g -O0 -ftest-coverage -fprofile-arcs)
target_link_options(${LIB_RAPTOR} PUBLIC --coverage)
target_link_libraries(${LIB_RAPTOR} PUBLIC ${LIB_MOCK})

add_executable(
  ${PROJECT_NAME}
  test_sysreg.cc
  test_utf8.cc
  test_scpi_err.cc
  test_lexer.cc
)
target_compile_options(${PROJECT_NAME} PUBLIC -g -O0)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB_RAPTOR} ${LIB_MOCK} GTest::gtest_main)

include(CTest)
include(GoogleTest)
include(CodeCoverage.cmake)

set(GCOVR_ADDITIONAL_ARGS "")
if (RAPTOR_COVERAGE_GCOV)
  message(STATUS "Adding custom gcov executable to gcovr: ${RAPTOR_COVERAGE_GCOV}")
  list(APPEND GCOVR_ADDITIONAL_ARGS --gcov-executable ${RAPTOR_COVERAGE_GCOV})
endif()
list(APPEND GCOVR_ADDITIONAL_ARGS --txt --txt-metric branch --gcov-ignore-parse-errors --exclude-unreachable-branches)
setup_target_for_coverage_gcovr_xml(
  NAME coverage
  EXECUTABLE ctest
  EXCLUDE "tests/unittests/*"
  BASE_DIRECTORY "../../"
  DEPENDANCIES ${LIB_RAPTOR}
)
gtest_discover_tests(${PROJECT_NAME})

