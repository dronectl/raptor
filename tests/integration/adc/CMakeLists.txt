cmake_minimum_required(VERSION 3.22)

set(CMAKE_BUILD_TYPE Debug)
# show fetch content progress
set(FETCHCONTENT_QUIET FALSE)
# Load toolchain files
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/../../../cmake/toolchains/stm32h723.cmake)
# specify module path
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../../cmake/modules ${CMAKE_MODULE_PATH})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)


project(
  adc-dma
  LANGUAGES C ASM
)

include(stm32cubeh7)
add_executable(
  ${PROJECT_NAME} 
  ${HAL_SRCS}
  src/adc.c
  src/dma.c
  src/gpio.c
  src/main.c
  src/tim.c
  src/stm32h7xx_hal_msp.c
  src/stm32h7xx_it.c
  src/syscalls.c
  src/sysmem.c
  src/system_stm32h7xx.c
  src/usart.c
  src/usb_otg.c
  src/startup_stm32h723xx.s
)
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${HAL_INC}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
message(STATUS "Adding STM32 compiler flags: ${STM32H723_COMPILE_FLAGS}")
target_compile_options(
  ${PROJECT_NAME}
  PUBLIC ${STM32H723_COMPILE_FLAGS}
)
target_link_options(
  ${PROJECT_NAME}
  PUBLIC -T${CMAKE_CURRENT_SOURCE_DIR}/STM32H723ZGTX_FLASH.ld ${STM32H723_COMPILE_FLAGS} -Wl,--print-memory-usage -Wl,--gc-sections -static -z muldefs -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--start-group -lc -lm -Wl,--end-group
)

include(openocd)
find_openocd()

message(STATUS "Adding custom targets")
add_custom_target(size ALL ${ARM_SIZE} -B -d ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})
add_custom_target(hex ALL ${ARM_OBJCPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME})
add_custom_target(bin ALL ${ARM_OBJCPY} -O binary -S ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin DEPENDS ${PROJECT_NAME})
add_custom_target(objdump ALL ${ARM_OBJDUMP} -h ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})
add_custom_target(flash DEPENDS size COMMAND ${OPENOCD_EXECUTABLE} -f${PROJECT_SOURCE_DIR}/stm32h723.cfg -c "program ${PROJECT_NAME}.elf verify reset exit")
add_custom_target(debug DEPENDS size COMMAND ${OPENOCD_EXECUTABLE} -f${PROJECT_SOURCE_DIR}/stm32h723.cfg -c "program ${PROJECT_NAME}.elf verify reset")

