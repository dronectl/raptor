# 
# Paths.cmake
#
# Populate CMake variables with paths to CGT, SDKs and other tools.
#

cmake_minimum_required(VERSION 3.22)

set(ARM_GNU_TOOLCHAIN_VERSION "13.2.Rel1" CACHE STRING "ARM GNU Toolchain version")
set(DARWIN_ARM_CGT_DEFAULT_PATH "/Applications/ArmGNUToolchain/${ARM_GNU_TOOLCHAIN_VERSION}/arm-none-eabi")
set(WINDOWS_ARM_CGT_DEFAULT_PATH "C:/Program Files/ArmGNUToolchain/${ARM_GNU_TOOLCHAIN_VERSION}/arm-none-eabi")
set(LINUX_ARM_CGT_DEFAULT_PATH "/usr/lib/arm-none-eabi-gcc/${ARM_GNU_TOOLCHAIN_VERSION}/arm-none-eabi")

function(paths_set_arm_gnu_cgt_path)
    if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
        set(ARM_GNU_CGT_PATH "${DARWIN_ARM_CGT_DEFAULT_PATH}" CACHE PATH "Path to ARM GNU Toolchain")
    elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        set(ARM_GNU_CGT_PATH "${WINDOWS_ARM_CGT_DEFAULT_PATH}" CACHE PATH "Path to ARM GNU Toolchain")
    elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
        set(ARM_GNU_CGT_PATH "${WINDOWS_ARM_CGT_DEFAULT_PATH}" CACHE PATH "Path to ARM GNU Toolchain")
    endif()
endfunction()

function(paths_set_openocd_executable)
    find_program(
        OPENOCD_EXECUTABLE
        NAMES openocd
        DOC "Path to an openocd executable"
    )
    if (NOT OPENOCD_EXECUTABLE)
        message(FATAL "Could not find an openocd program")
    endif()
endfunction()

function(paths_set_git_executable)
    find_program(
        GIT_EXECUTABLE
        NAMES git
        DOC "Path to a git executable"
    )
    if(NOT GIT_EXECUTABLE)
        message(FATAL "Could not find a git program")
    endif()
endfunction()
