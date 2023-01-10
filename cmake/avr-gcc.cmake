# ####
# CMake Toolchain for baremetal avr-gcc
# ####

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

# This avoids running the linker and is intended for use with cross-compiling toolchains
# that cannot link without custom flags or linker scripts.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(OBJECT_GEN_FLAGS "-O0 -Wno-unused-parameter -Wpedantic -fno-builtin -Wall -Wextra -fstack-usage -ffunction-sections -fdata-sections -fomit-frame-pointer")

set(CMAKE_C_FLAGS "${OBJECT_GEN_FLAGS}" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS}" CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp " CACHE INTERNAL "ASM Compiler options")

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage -Wl,--gc-sections" CACHE INTERNAL "Linker options")

set(CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE INTERNAL "C++ Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "-g" CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Linker options for debug build type")

set(CMAKE_C_FLAGS_RELEASE "-Os -flto" CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -flto" CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto" CACHE INTERNAL "Linker options for release build type")

# find avr toolchain programs
find_program(AVR_CC NAMES avr-gcc REQUIRED)
find_program(AVR_CXX NAMES avr-g++ REQUIRED)
find_program(AVR_OBJCOPY NAMES avr-objcopy REQUIRED)
find_program(AVR_OBJDUMP NAMES avr-objdump REQUIRED)
find_program(AVR_STRIP NAMES avr-strip REQUIRED)
find_program(AVR_SIZE NAMES avr-size REQUIRED)
find_program(AVRDUDE NAMES avrdude REQUIRED)

set(CMAKE_ASM_COMPILER ${AVR_CC})
set(CMAKE_C_COMPILER ${AVR_CC})
set(CMAKE_CXX_COMPILER ${AVR_CXX})