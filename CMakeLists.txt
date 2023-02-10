# 3.22 ubuntu-latest standard
cmake_minimum_required(VERSION 3.22)
# Load toolchain files
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# global project name
set(EXECUTABLE "raptor")
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)


# cmake build guard
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# cmake build guard
if(NOT DEFINED STM32CUBEF4_BASE)
  message(FATAL_ERROR "Must provide STM32CUBEF4 (absolute) base path: -DSTM32CUBEF4_BASE=/path/to/stm32cubef4")
endif()

# set default build type if none specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type was set. Setting build type to 'Release'")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose cmake build type: Debug Release" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()


project(
  ${EXECUTABLE}
  LANGUAGES C ASM
  VERSION 0.1.0
  DESCRIPTION "Hardware in the loop test executor and reporting aggregation system."
)



message(STATUS "Configuring config.h header")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in config.h @ONLY)

# Include the FreeRTOS source code
include(FetchContent)
set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE STRING "")
message(STATUS "Fetching FreeRTOS")
FetchContent_Declare(
  freertos_kernel
  GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
  GIT_TAG V10.5.1
)
add_library(freertos_config INTERFACE)
target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)
# Select the native compile PORT
set(FREERTOS_PORT "GCC_ARM_CM4F" CACHE STRING "" FORCE)
# Select the cross-compile PORT
if (CMAKE_CROSSCOMPILING)
  set(FREERTOS_PORT "GCC_ARM_CM4F" CACHE STRING "" FORCE)
endif()
FetchContent_MakeAvailable(freertos_kernel)

# set chip specific definitions
set(MCU STM32F411xE)

message(STATUS "Linking linker script.")
get_filename_component(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/tools/STM32F411VETx_FLASH.ld ABSOLUTE)

message(STATUS "Linking OpenOCD configuration.")
get_filename_component(OPENOCD_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/tools/stlink.cfg ABSOLUTE)

# set STM32F4xx include paths
set(
  STM32CUBEF4_INCLUDE
  ${STM32CUBEF4_BASE}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
  ${STM32CUBEF4_BASE}/Drivers/CMSIS/Core/Include
)

# set compile flags for STM32F411XE 
list(APPEND STM32F411XE_C_FLAGS -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
list(APPEND STM32F411XE_COMPILE_DEFINITIONS -D${MCU} -DSTM32F411VETx -DSTM32F411E_DISCO)
list(APPEND STM32F411XE_LINKER_FLAGS -T${LINKER_SCRIPT} -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)


# find code generation tooling
message(STATUS "Discovering cgt executables.")
find_program(ARM_OBJCPY NAMES ${TOOLCHAIN_PREFIX}-objcopy${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_DIR} NO_DEFAULT_PATH)
find_program(ARM_OBJDUMP NAMES ${TOOLCHAIN_PREFIX}-objdump${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_DIR} NO_DEFAULT_PATH)
find_program(ARM_SIZE NAMES ${TOOLCHAIN_PREFIX}-size${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_DIR} NO_DEFAULT_PATH)
find_program(OPENOCD NAMES openocd${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_DIR} NO_DEFAULT_PATH)


add_executable(
  ${EXECUTABLE}
  src/main.c
  src/cbuffer.c
  src/logger.c
  src/system_stm32f4xx.c
  src/startup_stm32f411vetx.s
)
set_target_properties(${EXECUTABLE} PROPERTIES OUTPUT_NAME ${EXECUTABLE}.elf)
target_include_directories(${EXECUTABLE} PUBLIC include)
target_link_libraries(${EXECUTABLE} PRIVATE freertos_kernel)
target_include_directories(${EXECUTABLE} PUBLIC ${STM32CUBEF4_INCLUDE})
target_compile_definitions(${EXECUTABLE} PRIVATE ${STM32F411XE_COMPILE_DEFINITIONS})
target_compile_options(${EXECUTABLE} PRIVATE ${STM32F411XE_C_FLAGS})
target_link_options(${EXECUTABLE} PRIVATE ${STM32F411XE_LINKER_FLAGS})




message(STATUS "Adding custom targets")
add_custom_target(size ALL ${ARM_SIZE} -B -d --target=binary ${EXECUTABLE}.elf DEPENDS ${EXECUTABLE})
add_custom_target(hex ALL ${ARM_OBJCPY} -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex DEPENDS ${EXECUTABLE})
add_custom_target(bin ALL ${ARM_OBJCPY} -O binary -S ${EXECUTABLE}.elf ${EXECUTABLE}.bin DEPENDS ${EXECUTABLE})
add_custom_target(objdump ${ARM_OBJDUMP} -S ${EXECUTABLE}.elf DEPENDS ${EXECUTABLE})
add_custom_target(flash DEPENDS size COMMAND ${OPENOCD} -finterface/stlink.cfg -ftarget/stm32f4x.cfg -c "program raptor.elf verify reset exit")