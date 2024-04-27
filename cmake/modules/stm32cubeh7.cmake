cmake_minimum_required(VERSION 3.14)
include(FetchContent)
message(STATUS "Fetching STM32H7 CMSIS HAL and middleware dependancies")
FetchContent_Declare(
  stm32h7
  GIT_REPOSITORY https://github.com/STMicroelectronics/STM32CubeH7.git
  GIT_TAG v1.11.2
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(stm32h7)

message(STATUS "Exporting FreeRTOS Src and Includes")
set(
  CMSIS_INCLUDE
  ${stm32h7_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
  ${stm32h7_SOURCE_DIR}/Drivers/CMSIS/Include
  ${stm32h7_SOURCE_DIR}/Drivers/CMSIS/Core/Include
  CACHE INTERNAL "CMSIS_INCLUDE"
)

set(
  FREERTOS_SRCS
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/croutine.c
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/list.c
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/queue.c
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/tasks.c
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/timers.c
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
  CACHE INTERNAL "FREERTOS_SRCS"
)
set(
  FREERTOS_INC
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/
  ${CMSIS_INCLUDE}
  CACHE INTERNAL "FREERTOS_INC"
)

message(STATUS "Exporting LWIP Src and Includes")
file(
  GLOB_RECURSE 
  LWIP_SRCS_TMP
  CONFIGURE_DEPENDS
  "${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/core/*.c"
  "${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/api/*.c"
  "${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/netif/ethernet.c"
  "${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system/OS/sys_arch.c"
)

set(
  LWIP_SRCS
  ${LWIP_SRCS_TMP}
  CACHE INTERNAL "LWIP_SRCS"
)
set(
  LWIP_INC
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system
  CACHE INTERNAL "LWIP_INC"
)

message(STATUS "Exporting HAL Src and Includes")
aux_source_directory(${stm32h7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src HAL_SRCS_TMP)
# Iterate over each source file in HAL_SRCS
foreach(src ${HAL_SRCS_TMP})
    string(FIND ${src} "template" HAS_TEMPLATE)
    if(${HAS_TEMPLATE} EQUAL -1)
        list(APPEND FILTERED_SRCS ${src})
    endif()
endforeach()

set(
  HAL_SRCS
  ${FILTERED_SRCS}
  ${stm32h7_SOURCE_DIR}/Drivers/BSP/Components/lan8742/lan8742.c
  ${stm32h7_SOURCE_DIR}/Drivers/BSP/STM32H7xx_Nucleo/stm32h7xx_nucleo.c
  CACHE INTERNAL "HAL_SRCS"
)
set(
  HAL_INC
  ${stm32h7_SOURCE_DIR}/Drivers/BSP/Components/lan8742
  ${stm32h7_SOURCE_DIR}/Drivers/BSP/STM32H7xx_Nucleo
  ${stm32h7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
  ${CMSIS_INCLUDE}
  CACHE INTERNAL "HAL_INC"
)

