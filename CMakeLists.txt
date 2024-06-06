cmake_minimum_required(VERSION 3.22)

option(RAPTOR_TESTS "Build core library unittests using host compilers" Off)
option(RAPTOR_COVERAGE "Add coverage symbols" Off)

# show fetch content progress
set(FETCHCONTENT_QUIET FALSE)
# Load toolchain files
if (NOT RAPTOR_TESTS)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains/stm32h723.cmake)
endif()

# specify module path
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# global names for downstream libs
set(LIB_BSP "lib-bsp")
set(LIB_RAPTOR "lib-raptor")


project(
  raptor
  LANGUAGES C ASM CXX
  VERSION 0.1.0
  DESCRIPTION "UAV propulsion test executor."
)

# cmake build guard
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# set default build type if none specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type was set. Setting build type to 'Release'")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose cmake build type: Debug Release" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

set(CMAKE_STATIC_LIBRARY_PREFIX "")

include(stm32cubeh7)
fetch_cubeh7()

if (NOT RAPTOR_TESTS)
  include(openocd)
  find_openocd()

endif()

# project config
message(STATUS "Configuring config.h header")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


if (RAPTOR_TESTS)
  add_subdirectory(tests)
else()
  add_subdirectory(bsp)
  add_subdirectory(core)
endif()

