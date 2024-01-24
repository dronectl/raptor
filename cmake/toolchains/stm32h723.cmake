##
## STM32H723 Toolchain
## 
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm-eabi)
set(TOOLCHAIN_PREFIX arm-none-eabi)
# get the binary search command for the host platform and set toolchain extensions
if(MINGW OR CYGWIN OR WIN32)
  find_program(
    CMAKE_MAKE_PROGRAM
    NAMES make
          make.exe
    DOC "Find a suitable make program for building under Windows/MinGW"
    HINTS c:/MinGW-32/bin
  )
  set(UTIL_SEARCH_CMD where.exe)
  set(TOOLCHAIN_EXT ".exe" )
elseif(UNIX OR APPLE)
  set(UTIL_SEARCH_CMD which)
  set(TOOLCHAIN_EXT "" )
endif()

# search for arm toolchain binary and get its absolute path
execute_process(
  COMMAND ${UTIL_SEARCH_CMD} ${TOOLCHAIN_PREFIX}-gcc
  OUTPUT_VARIABLE BINUTILS_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# add handler for unfound resources
if (${BINUTILS_PATH} STREQUAL "")
  message(FATAL "Could not locate ${TOOLCHAIN_PREFIX} compiler and utilities.")
endif()

# get the filename components path
get_filename_component(TOOLCHAIN_DIR ${BINUTILS_PATH} DIRECTORY)
message(STATUS "Discovered ARM CGT path: " ${TOOLCHAIN_DIR})

# This avoids running the linker and is intended for use with cross-compiling toolchains
# that cannot link without custom flags or linker scripts.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# set MCU definition for CMSIS headers
set(STM32H723_MCU "STM32H723xx" CACHE INTERNAL "MCU model")
set(STM32H723_ARM_ARCH "cortex-m7" CACHE INTERNAL "ARM architecture")
# single-precision and double-precision FPU with 16 double-word registers
# From PM0253 programmers manual Section 4.7 Floating-point unit
set(STM32H723_FPU_ARCH "fpv5-d16" CACHE INTERNAL "FPU architecture")
# use standard ARM Architecture Procedure Call Standard ABI
set(STM32H723_ABI "aapcs" CACHE INTERNAL "Compiler ABI")

# configure compile commands for CMSIS and compiler
# use hardware fpu and use mthumb optimization (does not affect debug mode)
set(STM32H723_COMPILE_FLAGS "-D${STM32H723_MCU} -DUSE_HAL_DRIVER -mcpu=${STM32H723_ARM_ARCH} -mfpu=${STM32H723_FPU_ARCH} -mabi=${STM32H723_ABI} -mfloat-abi=hard -mthumb")
message(STATUS "Exporting linker script path")
get_filename_component(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/utils/STM32H723ZGTX_FLASH.ld ABSOLUTE)
message(DEBUG "Linker script: ${LINKER_SCRIPT}")
set(STM32H723_LINKER_FLAGS "-T${LINKER_SCRIPT} ${STM32H723_COMPILE_FLAGS} -Wl,--print-memory-usage -Wl,--gc-sections --specs=nosys.specs --specs=nano.specs -static -z muldefs -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--start-group -lc -lm -Wl,--end-group" CACHE INTERNAL "STM32H723 linker options")
message(STATUS "Setting compilers.")
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc${TOOLCHAIN_EXT} CACHE INTERNAL "C Compiler")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++${TOOLCHAIN_EXT} CACHE INTERNAL "C++ Compiler")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}-gcc${TOOLCHAIN_EXT} CACHE INTERNAL "ASM Compiler")

message(STATUS "Discovering ARM CGT executables.")
find_program(ARM_OBJCPY NAMES ${TOOLCHAIN_PREFIX}-objcopy${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_DIR} NO_DEFAULT_PATH)
find_program(ARM_OBJDUMP NAMES ${TOOLCHAIN_PREFIX}-objdump${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_DIR} NO_DEFAULT_PATH)
find_program(ARM_SIZE NAMES ${TOOLCHAIN_PREFIX}-size${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_DIR} NO_DEFAULT_PATH)

# configure find_program search directives
set(CMAKE_FIND_ROOT_PATH ${BINUTILS_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)