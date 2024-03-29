cmake_minimum_required(VERSION 3.22)
set(FETCHCONTENT_QUIET FALSE)
# Load toolchain files
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains/stm32h723.cmake)
# specify module path
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# For debug, no change detected ...
# set(CMAKE_VERBOSE_MAKEFILE ON)
# global project name
set(EXECUTABLE "raptor")
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(
  ${EXECUTABLE}
  LANGUAGES C ASM
  VERSION 0.1.0
  DESCRIPTION "UAV propulsion test executor."
)

include(cubeh7)
include(openocd)
include(freertos)
find_openocd()


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
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_adc.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_adc_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_rcc.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_rcc_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_gpio.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_dma.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_dma_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_mdma.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_hsem.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_wwdg.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_fdcan.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_exti.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_eth_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_eth.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_rtc.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_rtc_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_uart.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_uart_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_cortex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_flash.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_flash_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_pwr.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_pwr_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_spi.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_spi_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_i2c.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_i2c_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_sd.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_sd_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_tim.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_tim_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_ll_sdmmc.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_ll_delayblock.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal.c
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

set(GCC_FLAGS -Wno-unused-parameter -Wpedantic -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections -fomit-frame-pointer)
message(STATUS "Appending GCC flags: ${GCC_FLAGS}")
target_compile_options(
  ${EXECUTABLE}
  PRIVATE ${GCC_FLAGS} ${STM32H723_COMPILE_FLAGS}
  PRIVATE $<$<CONFIG:Debug>:-DRAPTOR_DEBUG -O0 -g>
  PRIVATE $<$<CONFIG:Release>:-Os>
)
target_link_options(
  ${EXECUTABLE}
  PRIVATE ${STM32H723_LINKER_FLAGS}
  PRIVATE $<$<CONFIG:Debug>:-DRAPTOR_DEBUG -O0 -g>
  PRIVATE $<$<CONFIG:Release>:-Os>
)

# only include tests if BUILD_TESTS is enabled
option(BUILD_TESTS "Build tests" ON)
if (BUILD_TESTS)
  message(STATUS "Adding test binaries to build")
  add_subdirectory(tests)
endif()

message(STATUS "Adding custom targets")
add_custom_target(size ALL ${ARM_SIZE} -B -d --target=binary ${EXECUTABLE}.elf DEPENDS ${EXECUTABLE})
add_custom_target(hex ALL ${ARM_OBJCPY} -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex DEPENDS ${EXECUTABLE})
add_custom_target(bin ALL ${ARM_OBJCPY} -O binary -S ${EXECUTABLE}.elf ${EXECUTABLE}.bin DEPENDS ${EXECUTABLE})
add_custom_target(objdump ${ARM_OBJDUMP} -S ${EXECUTABLE}.elf DEPENDS ${EXECUTABLE})
add_custom_target(flash DEPENDS size COMMAND ${OPENOCD_EXECUTABLE} -f../utils/stm32h723.cfg -c "program ${EXECUTABLE}.elf verify reset exit")
