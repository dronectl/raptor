cmake_minimum_required(VERSION 3.22)

# create third party libraries
add_library(lib-freertos STATIC ${FREERTOS_SRCS})
target_include_directories(lib-freertos PUBLIC ${FREERTOS_INC} freertos)
target_link_libraries(lib-freertos ${LIB_BSP})
add_library(lib-lwip STATIC ${LWIP_SRCS})
target_include_directories(lib-lwip PUBLIC ${LWIP_INC} lwip)
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
  lib/system.c
  lib/scpi/utf8.c
  lib/scpi/lexer.c
  lib/scpi/parser.c
  lib/scpi/commands.c
  lib/scpi/server.c
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
  PUBLIC $<$<CONFIG:Debug>:-DRAPTOR_DEBUG>
)
target_link_libraries(
  ${LIB_RAPTOR}
  ${LIB_BSP}
  lib-freertos
  lib-lwip
)

add_executable(
  ${PROJECT_NAME}
  src/startup.c
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

# inherit compile flags from libraries
target_link_libraries(${PROJECT_NAME} ${LIB_BSP} ${LIB_RAPTOR} lib-freertos lib-lwip)

message(STATUS "Adding custom targets")
add_custom_target(size ALL ${ARM_SIZE} -B -d --target=binary ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})
add_custom_target(hex ALL ${ARM_OBJCPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME})
add_custom_target(bin ALL ${ARM_OBJCPY} -O binary -S ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin DEPENDS ${PROJECT_NAME})
add_custom_target(objdump ${ARM_OBJDUMP} -h ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})
add_custom_target(flash DEPENDS size COMMAND ${OPENOCD_EXECUTABLE} -f${PROJECT_SOURCE_DIR}/utils/stm32h723.cfg -c "program ${PROJECT_NAME}.elf verify reset exit")

