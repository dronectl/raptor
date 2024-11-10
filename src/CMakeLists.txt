cmake_minimum_required(VERSION 3.22)

add_subdirectory(bsp)

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
target_include_directories(lib-freertos PUBLIC ${FREERTOS_INC} common/config)
target_link_libraries(lib-freertos ${LIB_BSP})
add_library(lib-lwip STATIC ${LWIP_SRCS})
target_include_directories(lib-lwip PUBLIC ${LWIP_INC} common/config)
target_link_libraries(lib-lwip lib-freertos)

message(STATUS "Consolidating files for ${LIB_RAPTOR}")
add_library(
  ${LIB_RAPTOR}
  STATIC
  # TODO: decouple application from driver code: https://github.com/dronectl/raptor/issues/66
  drivers/ethernet/app_ethernet.c
  drivers/ethernet/ethernetif.c
  drivers/bme280.c
  drivers/pwm.c
  common/cbuffer.c
  common/logger.c
  common/sysreg.c
  os/health.c
  os/system.c
  os/scpi/err.c
  os/scpi/utf8.c
  os/scpi/lexer.c
  os/scpi/parser.c
  os/scpi/commands.c
  os/scpi/server.c
)

target_include_directories(${LIB_RAPTOR} PUBLIC drivers os common)

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
  main.c
  os/freertos.c
  os/sysmem.c
  os/syscalls.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE drivers
  PRIVATE os
  PRIVATE common
  PRIVATE ${FREERTOS_INC}
  PRIVATE ${LWIP_INC}
)

# inherit compile flags from libraries
target_link_libraries(${PROJECT_NAME} ${LIB_BSP} ${LIB_RAPTOR} lib-freertos lib-lwip)


