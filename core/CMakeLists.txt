cmake_minimum_required(VERSION 3.22)

# create third party libraries
add_library(
  lib-freertos 
  STATIC
  ${FREERTOS_SRCS}
)
target_include_directories(
  lib-freertos
  PUBLIC
  ${FREERTOS_INC}
  freertos
)
target_compile_options(lib-freertos PUBLIC ${STM32H723_COMPILE_FLAGS})
target_link_options(lib-freertos PUBLIC ${STM32H723_LINKER_FLAGS})
target_link_libraries(lib-freertos ${LIB_BSP})
add_library(
  lib-lwip
  STATIC
  ${LWIP_SRCS}
)
target_include_directories(
  lib-lwip
  PUBLIC
  ${LWIP_INC}
  lwip
)
target_compile_options(lib-lwip PUBLIC ${STM32H723_COMPILE_FLAGS})
target_link_options(lib-lwip PUBLIC ${STM32H723_LINKER_FLAGS})
target_link_libraries(lib-lwip lib-freertos)

message(STATUS "Consolidating files for ${LIB_RAPTOR}")
add_library(
  ${LIB_RAPTOR}
  STATIC
  drivers/bme280.c
  drivers/pwm.c
  lib/health.c
  lib/cbuffer.c
  lib/logger.c
  lib/sysreg.c
)
target_include_directories(
  ${LIB_RAPTOR}
  PUBLIC
  drivers
  lib
)
target_compile_options(
  ${LIB_RAPTOR}
  PUBLIC -Wno-unused-parameter -Wpedantic -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections -fomit-frame-pointer
  PUBLIC ${STM32H723_COMPILE_FLAGS}
  PUBLIC $<$<CONFIG:Debug>:-DRAPTOR_DEBUG -O0 -g>
  PUBLIC $<$<CONFIG:Release>:-O3>
)
target_link_libraries(
  ${LIB_RAPTOR}
  ${LIB_BSP}
  lib-freertos
  lib-lwip
)
target_link_options(
  ${LIB_RAPTOR}
  PUBLIC
  ${STM32H723_LINKER_FLAGS}
)

add_executable(
  ${PROJECT_NAME}
  src/startup.c
  src/startup_stm32h723zgtx.s
  lwip/app_ethernet.c
  lwip/ethernetif.c
  freertos/freertos.c
  freertos/sysmem.c
  freertos/syscalls.c
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE drivers
  PRIVATE freertos
  PRIVATE lwip
  PRIVATE src
  PRIVATE ${FREERTOS_INC}
  PRIVATE ${LWIP_INC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.elf)

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE -Wno-unused-parameter -Wpedantic -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections -fomit-frame-pointer
  PRIVATE ${STM32H723_COMPILE_FLAGS}
  PRIVATE $<$<CONFIG:Debug>:-DRAPTOR_DEBUG -O0 -g>
  PRIVATE $<$<CONFIG:Release>:-O3>
)

target_link_libraries(${PROJECT_NAME} ${LIB_BSP} lib-freertos lib-lwip)
target_link_options(
  ${PROJECT_NAME}
  PRIVATE ${STM32H723_LINKER_FLAGS}
)

target_link_libraries(${PROJECT_NAME} ${LIB_RAPTOR})

