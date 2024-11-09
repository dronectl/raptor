cmake_minimum_required(VERSION 3.22)

# protocols nanopb
message(STATUS "Building raptor protocols")
include(nanopb)
nanopb_generate_cpp(
  TARGET lib-protocols
  RELPATH protocols/protobuf
  protocols/protobuf/raptor/v1/battery.proto
  protocols/protobuf/raptor/v1/bldc.proto
  protocols/protobuf/raptor/v1/env.proto
  protocols/protobuf/raptor/v1/metadata.proto
  protocols/protobuf/raptor/v1/raptor.proto
  protocols/protobuf/raptor/v1/relay.proto
)

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
  lib/scpi/err.c
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
  lib-protocols
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

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

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


