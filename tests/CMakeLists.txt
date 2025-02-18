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
set(PROJECT_ROOT "..")

include(CTest)
include(GoogleTest)
include(CodeCoverage.cmake)
include(stm32cubeh7)

# configure headers as global system includes to supress cross-compile warnings
include_directories(
  SYSTEM
  mock
  ${HAL_INC}
  ${LWIP_INC}
  ${FREERTOS_INC}
  ${PROJECT_ROOT}/src/common/config
  ${PROJECT_ROOT}/src/os
  ${PROJECT_ROOT}/src/common
  ${PROJECT_ROOT}/src/drivers
  ${PROJECT_ROOT}/src/bsp/inc
)

# global test compile options
add_compile_definitions(UNITTEST STM32H723xx USE_HAL_DRIVER)
add_compile_options(-g -O0 -ftest-coverage -fprofile-arcs -fpermissive)

# test entry helper
set(REGISTERED_TESTS "")
function(add_gtest test_name source_file)
  add_executable(${test_name} ${source_file} ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.cc)
  target_link_libraries(${test_name} PRIVATE GTest::gtest_main GTest::gmock)
  add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
  set(REGISTERED_TESTS ${REGISTERED_TESTS} ${test_name} PARENT_SCOPE)
endfunction()

# add tests here
add_gtest(test_hsm ${PROJECT_ROOT}/src/os/hsm.c)
add_gtest(test_sysreg ${PROJECT_ROOT}/src/common/sysreg.c)

message(STATUS "Registered tests: ${REGISTERED_TESTS}")

# coverage configuration
set(GCOVR_ADDITIONAL_ARGS "")
if (RAPTOR_COVERAGE_GCOV)
  message(STATUS "Adding custom gcov executable to gcovr: ${RAPTOR_COVERAGE_GCOV}")
  list(APPEND GCOVR_ADDITIONAL_ARGS --gcov-executable ${RAPTOR_COVERAGE_GCOV})
endif()

message(STATUS "Adding custom gcov executable to gcovr: ${RAPTOR_COVERAGE_GCOV}")
list(APPEND GCOVR_ADDITIONAL_ARGS --txt --txt-metric branch --gcov-ignore-parse-errors --exclude-unreachable-branches)
setup_target_for_coverage_gcovr_xml(
  NAME coverage
  EXECUTABLE ctest
  EXCLUDE "tests/*"
  BASE_DIRECTORY "../"
  DEPENDS ${REGISTERED_TESTS}
)

