cmake_minimum_required(VERSION 3.22)

add_executable(
  ${PROJECT_NAME}
  src/startup.c
  src/startup_stm32h723zgtx.s
  lwip/app_ethernet.c
  lwip/ethernetif.c
  drivers/bme280.c
  drivers/pwm.c
  lib/health.c
  lib/cbuffer.c
  lib/logger.c
  lib/sysreg.c
  freertos/freertos.c
  freertos/sysmem.c
  freertos/syscalls.c
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE include
  PRIVATE ${STM32CUBEH7_CMSIS_INCLUDE}
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.elf)

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE -Wno-unused-parameter -Wpedantic -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections -fomit-frame-pointer
  PRIVATE $<$<BOOL:NOT ${RAPTOR_TESTS}>${STM32H723_COMPILE_FLAGS}>
  PRIVATE $<$<CONFIG:Debug>:-DRAPTOR_DEBUG -O0 -g>
  PRIVATE $<$<CONFIG:Release>:-O3>
)

target_link_options(
  ${PROJECT_NAME}
  PRIVATE $<$<BOOL:NOT ${RAPTOR_TESTS}>${STM32H723_LINKER_FLAGS}>
)

target_link_libraries(${PROJECT_NAME} ${STM32CUBEH7_LIB})

