cmake_minimum_required(VERSION 3.22)

message(DEBUG "Entering bsp")

include(stm32cubeh7)

set(BSP_SRCS "")
aux_source_directory(Core/Src BSP_SRCS)
add_library(
  ${LIB_BSP}
  STATIC
  ${BSP_SRCS}
  ${HAL_SRCS}
)
target_include_directories(
  ${LIB_BSP}
  PUBLIC
  ${HAL_INC}
  Core/Inc
)
target_compile_options(${LIB_BSP} PUBLIC ${STM32H723_COMPILE_FLAGS})
target_link_options(${LIB_BSP} PUBLIC ${STM32H723_LINKER_FLAGS})

message(DEBUG "Finishing bsp")
