cmake_minimum_required(VERSION 3.22)

# build options
option(FLASH_TARGET "Create firmware flash target" ON)

include(Paths)
include(Version)

paths_set_arm_gnu_cgt_path()

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains/stm32h723.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(
    raptor
    LANGUAGES C ASM
    VERSION 0.1.0
    DESCRIPTION "UAV propulsion test executor."
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

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

version_file_configure(${PROJECT_VERSION_MAJOR} ${PROJECT_VERSION_MINOR} ${PROJECT_VERSION_PATCH})

add_subdirectory(src)

message(STATUS "Adding custom targets")
add_custom_target(size ALL ${ARM_SIZE} -B -d ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})
add_custom_target(hex ALL ${ARM_OBJCPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME})
add_custom_target(bin ALL ${ARM_OBJCPY} -O binary -S ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin DEPENDS ${PROJECT_NAME})
add_custom_target(objdump ALL ${ARM_OBJDUMP} -h ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})

if (${FLASH_TARGET})
    message(STATUS "Adding Openocd flash target")
    paths_set_openocd()
    add_custom_target(flash DEPENDS size COMMAND ${OPENOCD_EXECUTABLE} -f${PROJECT_SOURCE_DIR}/utils/stm32h723.cfg -c "program ${PROJECT_NAME}.elf verify reset exit")
endif()

