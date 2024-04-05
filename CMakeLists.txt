cmake_minimum_required(VERSION 3.22)

# show fetch content progress
set(FETCHCONTENT_QUIET FALSE)
# Load toolchain files
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains/stm32h723.cmake)
# specify module path
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# For debug, no change detected ...
# set(CMAKE_VERBOSE_MAKEFILE ON)
# global project name

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(
  raptor
  LANGUAGES C ASM
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

include(openocd)
include(freertos)
find_openocd()

# project config
message(STATUS "Configuring config.h header")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# build stm32cubeh7 lib
include(stm32cubeh7)
# build raptor lib
set(RAPTOR_LIB libraptor)
add_subdirectory(lib)

add_executable(
  ${PROJECT_NAME}
  src/main.c
  src/lwip/app_ethernet.c
  src/lwip/ethernetif.c
  src/drivers/bme280.c
  src/drivers/pwm.c
  src/health.c
  src/cbuffer.c
  src/logger.c
  src/freertos.c
  src/stcore/sysmem.c
  src/stcore/syscalls.c
  src/stcore/stm32h7xx_it.c
  src/stcore/stm32h7xx_hal_msp.c
  src/stcore/stm32h7xx_hal_timebase_tim.c
  src/stcore/startup_stm32h723zgtx.s
  src/stcore/system_stm32h7xx.c
)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE ${STM32CUBEH7_CMSIS_INCLUDE})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.elf)

target_compile_options(
  ${RAPTOR_LIB}
  PUBLIC -Wno-unused-parameter -Wpedantic -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections -fomit-frame-pointer ${STM32H723_COMPILE_FLAGS}
  PUBLIC $<$<CONFIG:Debug>:-DRAPTOR_DEBUG -O0 -g>
  PUBLIC $<$<CONFIG:Release>:-O3>
)

target_link_options(
  ${RAPTOR_LIB}
  PUBLIC
  ${STM32H723_LINKER_FLAGS}
)

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE -Wno-unused-parameter -Wpedantic -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections -fomit-frame-pointer ${STM32H723_COMPILE_FLAGS}
  PRIVATE $<$<CONFIG:Debug>:-DRAPTOR_DEBUG -O0 -g>
  PRIVATE $<$<CONFIG:Release>:-O3>
)

target_link_options(
  ${PROJECT_NAME}
  PRIVATE
  ${STM32H723_LINKER_FLAGS}
)
target_link_libraries(${PROJECT_NAME} ${RAPTOR_LIB} ${STM32CUBEH7_LIB})

if (RAPTOR_TESTS)
  message(STATUS "Including raptor tests")
  add_subdirectory(tests)
endif()

message(STATUS "Adding custom targets")
add_custom_target(size ALL ${ARM_SIZE} -B -d --target=binary ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})
add_custom_target(hex ALL ${ARM_OBJCPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME})
add_custom_target(bin ALL ${ARM_OBJCPY} -O binary -S ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin DEPENDS ${PROJECT_NAME})
add_custom_target(objdump ${ARM_OBJDUMP} -S ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})
add_custom_target(flash DEPENDS size COMMAND ${OPENOCD_PROJECT_NAME} -f${PROJECT_SOURCE_DIR}/utils/stm32h723.cfg -c "program ${PROJECT_NAME}.elf verify reset exit")

