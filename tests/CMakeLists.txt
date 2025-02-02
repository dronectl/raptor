cmake_minimum_required(VERSION 3.22)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules ${CMAKE_MODULE_PATH})
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

include(CTest)
include(GoogleTest)
include(CodeCoverage.cmake)
include(stm32cubeh7)

# configure headers as global system includes to supress cross-compile warnings
include_directories(SYSTEM ${HAL_INC} ${LWIP_INC} ${FREERTOS_INC} ../src/common/config ../src/os ../src/common ../src/drivers ../src/bsp/inc)

add_executable(test_hsm ../src/os/hsm.c test_hsm.cc)
target_include_directories(test_hsm PRIVATE mock)
target_link_libraries(test_hsm PRIVATE GTest::gtest_main GTest::gmock)
target_compile_definitions(test_hsm PUBLIC UNITTEST STM32H723xx USE_HAL_DRIVER)
target_compile_options(test_hsm PUBLIC -g -O0 -ftest-coverage -fprofile-arcs -fpermissive)

# Add tests to ctest
add_test(
  NAME test_hsm
  COMMAND $<TARGET_FILE:test_hsm>
)

set(GCOVR_ADDITIONAL_ARGS "")
if (RAPTOR_COVERAGE_GCOV)
  message(STATUS "Adding custom gcov executable to gcovr: ${RAPTOR_COVERAGE_GCOV}")
  list(APPEND GCOVR_ADDITIONAL_ARGS --gcov-executable ${RAPTOR_COVERAGE_GCOV})
  list(APPEND GCOVR_ADDITIONAL_ARGS --txt --txt-metric branch --gcov-ignore-parse-errors --exclude-unreachable-branches)
  setup_target_for_coverage_gcovr_xml(
    NAME coverage
    EXECUTABLE ctest
    EXCLUDE "tests/*"
    BASE_DIRECTORY "../"
    DEPENDANCIES ../src/os/hsm.c
  )
endif()


