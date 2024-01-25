cmake_minimum_required(VERSION 3.22)
# Load toolchain files
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains/stm32h723.cmake)
# specify module path
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# For debug, no change detected ...
set(CMAKE_VERBOSE_MAKEFILE ON)
# global project name
set(EXECUTABLE "raptor")
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(
  ${EXECUTABLE}
  LANGUAGES C ASM
  VERSION 0.1.0
  DESCRIPTION "Propulsion test executor."
)

include(cubeh7)
include(openocd)
include(freertos)

set(GCC_FLAGS "-Wno-unused-parameter -Wpedantic -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections -fomit-frame-pointer")
message(STATUS "Appending GCC flags: ${GCC_FLAGS}")
set(CMAKE_C_FLAGS   "${STM32H723_COMPILE_FLAGS} ${GCC_FLAGS}")
set(CMAKE_ASM_FLAGS "${STM32H723_COMPILE_FLAGS} ${GCC_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${STM32H723_LINKER_FLAGS}")

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

# Options for DEBUG build
set(CMAKE_C_FLAGS_DEBUG "-DRAPTOR_DEBUG -O0 -g" CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "-O0 -g" CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-O0 -g" CACHE INTERNAL "Linker options for debug build type")

# Options for RELEASE build
set(CMAKE_C_FLAGS_RELEASE "-Os" CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "-Os" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Os" CACHE INTERNAL "Linker options for release build type")

# project config
message(STATUS "Configuring config.h header")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

fetch_cubeh7()

add_executable(
  ${EXECUTABLE}
  src/main.c
  src/lwip/app_ethernet.c
  src/lwip/ethernetif.c
  src/syscalls.c
  src/drivers/hx711.c
  src/drivers/bme280.c
  src/sysmem.c
  src/health.c
  src/freertos_openocd.c
  src/stm32h7xx_it.c
  src/stm32h7xx_hal_msp.c
  src/startup_stm32h723zgtx.s
  src/system_stm32h7xx.c
  ${STM32CUBEH7_SRC}/Drivers/BSP/Components/lan8742/lan8742.c
  ${STM32CUBEH7_SRC}/Drivers/BSP/STM32H7xx_Nucleo/stm32h7xx_nucleo.c
  ${LWIP_SRC}
  ${STM32CUBEH7_HAL_SRC}
  ${STM32CUBEH7_FREERTOS_SRC}
)
target_include_directories(${EXECUTABLE} PRIVATE ${LWIP_INCLUDE})
target_include_directories(${EXECUTABLE} PRIVATE ${STM32CUBEH7_FREERTOS_INCLUDE})
target_include_directories(${EXECUTABLE} PRIVATE ${STM32CUBEH7_HAL_INCLUDE})
target_include_directories(${EXECUTABLE} PRIVATE ${STM32CUBEH7_BSP_INCLUDE})
target_include_directories(${EXECUTABLE} PRIVATE include)
target_include_directories(${EXECUTABLE} PRIVATE ${STM32CUBEH7_CMSIS_INCLUDE})
set_target_properties(${EXECUTABLE} PROPERTIES OUTPUT_NAME ${EXECUTABLE}.elf)

if(BUILD_TESTS)
  message(STATUS "Adding test directory to build.")
  add_subdirectory(tests)
endif()

find_openocd()

message(STATUS "Adding custom targets")
add_custom_target(size ALL ${ARM_SIZE} -B -d --target=binary ${EXECUTABLE}.elf DEPENDS ${EXECUTABLE})
add_custom_target(hex ALL ${ARM_OBJCPY} -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex DEPENDS ${EXECUTABLE})
add_custom_target(bin ALL ${ARM_OBJCPY} -O binary -S ${EXECUTABLE}.elf ${EXECUTABLE}.bin DEPENDS ${EXECUTABLE})
add_custom_target(objdump ${ARM_OBJDUMP} -S ${EXECUTABLE}.elf DEPENDS ${EXECUTABLE})
add_custom_target(flash DEPENDS size COMMAND ${OPENOCD_EXECUTABLE} -f../utils/stm32h723.cfg -c "program ${EXECUTABLE}.elf verify reset exit")
