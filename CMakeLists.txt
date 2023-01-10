cmake_minimum_required(VERSION 3.22)

# always export compile commands for editor intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/avr-gcc.cmake)

# set c standard
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)

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

# global project name
set(PROJECT_NAME "raptor")

project(
  ${PROJECT_NAME}
  LANGUAGES C ASM
  VERSION "0.1"
  DESCRIPTION "Hardware in the loop test executor and reporting aggregation system"
)

message(STATUS "Adding source files")
add_executable(
  ${PROJECT_NAME}
  src/main.c
)

# AVR Fuses
set(E_FUSE 0xfd)
set(H_FUSE 0xd8)
set(L_FUSE 0xff)
set(LOCK_BIT 0xcf)

message(STATUS "--------------------------------------------------------------")
message(STATUS "Fuse Settings:")
message(STATUS "E_FUSE: " ${E_FUSE})
message(STATUS "H_FUSE: " ${H_FUSE})
message(STATUS "L_FUSE: " ${L_FUSE})
message(STATUS "LOCK_BIT: " ${LOCK_BIT})

# # AVR Chip Configuration
set(F_CPU 16000000UL)
set(MCU atmega328p)
set(AVRDUDE_MCU_CODE m328p)

# Default Baudrate for UART, read avr include/util/setbaud.h for usage
set(BAUDRATE 115200)

# The programmer to use, read avrdude manual for list
set(AVRDUDE_PROG_TYPE "arduino")

# set serial usb device for flashing
set(SERIAL_PORT "/dev/tty.usbserial-A6024AOD")

message(STATUS "--------------------------------------------------------------")
message(STATUS "MCU Settings:")
message(STATUS "MCU: " ${MCU})
message(STATUS "F_CPU: " ${F_CPU})
message(STATUS "BAUD: " ${BAUDRATE})
message(STATUS "SERIAL_PORT: " ${SERIAL_PORT})
message(STATUS "--------------------------------------------------------------")

message(STATUS "Configuring config.h header")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in config.h @ONLY)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.elf)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE "include")
target_compile_options(${PROJECT_NAME} PRIVATE -mmcu=${MCU} -Wno-main -Wundef -Wstrict-prototypes -Werror -Wfatal-errors -gdwarf-2 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fno-split-wide-types -fno-tree-scev-cprop)
target_link_options(${PROJECT_NAME} PRIVATE -mmcu=${MCU} -u vfprintf -lprintf_flt -lm)

target_compile_definitions(${PROJECT_NAME} PRIVATE -DF_CPU=${F_CPU})
message(STATUS "Adding custom targets")

# Strip binary for upload
add_custom_target(strip ALL ${AVR_STRIP} ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})

# Transform binary into hex file, we ignore the eeprom segments in the step
add_custom_target(hex ALL ${AVR_OBJCOPY} -R .eeprom -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS strip)

# target for firmware size details
add_custom_target(size ALL ${AVR_SIZE} ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})

# target for disassembly dump
add_custom_target(objdump ${AVR_OBJDUMP} -S ${PROJECT_NAME}.elf DEPENDS ${PROJECT_NAME})

# Upload the firmware with avrdude
add_custom_target(flash ${AVRDUDE} -c${AVRDUDE_PROG_TYPE} -p${MCU} -P${SERIAL_PORT} -b${BAUDRATE} -D -Uflash:w:${PROJECT_NAME}.hex:i DEPENDS hex)

# Burn fuses
add_custom_target(fuses ${AVRDUDE} -c${AVRDUDE_PROG_TYPE} -p${MCU} -P${SERIAL_PORT} -b${BAUDRATE} -Ulfuse:w:${L_FUSE}:m -U hfuse:w:${H_FUSE}:m -U efuse:w:${E_FUSE}:m -U lock:w:${LOCK_BIT}:m)

message(STATUS "Flagging additional files for cleaning")
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex;${PROJECT_NAME}.eeprom;${PROJECT_NAME}.lst")