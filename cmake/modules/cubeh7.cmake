cmake_minimum_required(VERSION 3.22)

# Fetch cube h7 headers and middlewares
function(fetch_cubeh7)
  include(FetchContent)
  message(STATUS "Fetching STM32H7 CMSIS HAL and middleware dependancies")
  FetchContent_Declare(
    stm32h7
    GIT_REPOSITORY https://github.com/STMicroelectronics/STM32CubeH7.git
    GIT_TAG v1.11.0
  )
  FetchContent_MakeAvailable(stm32h7)
  set(
    STM32CUBEH7_CMSIS_INCLUDE
    ${stm32h7_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${stm32h7_SOURCE_DIR}/Drivers/CMSIS/Core/Include
    ${stm32h7_SOURCE_DIR}/Drivers/CMSIS/Include
    CACHE INTERNAL "STM32CUBEH7_CMSIS_INCLUDE"
  )
  set(
    STM32CUBEH7_FREERTOS_INCLUDE
    ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/
    CACHE INTERNAL "STM32CUBEH7_FREERTOS_INCLUDE"
  )
  set(
    STM32CUBEH7_FREERTOS_SRC
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
    CACHE INTERNAL "STM32CUBEH7_FREERTOS_SRC"
  )
  set(
    STM32CUBEH7_HAL_INCLUDE
    ${STM32CUBEH7_CMSIS_INCLUDE}
    ${stm32h7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
    CACHE INTERNAL "STM32CUBEH7_HAL_INCLUDE"
  )
  set(
    STM32CUBEH7_BSP_INCLUDE
    ${stm32h7_SOURCE_DIR}/Drivers/BSP/Components/lan8742
    ${stm32h7_SOURCE_DIR}/Drivers/BSP/STM32H7xx_Nucleo
    CACHE INTERNAL "STM32CUBEH7_BSP_INCLUDE"
  )
  set(STM32CUBEH7_SRC ${stm32h7_SOURCE_DIR} PARENT_SCOPE)
  set(
    LWIP_INCLUDE
    ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include
    ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system
    CACHE INTERNAL "LWIP_INCLUDE"
  )
  file(
    GLOB_RECURSE 
    LWIP_SRC
    CONFIGURE_DEPENDS
    "${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/core/*.c"
    "${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/api/*.c"
    "${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/netif/ethernet.c"
    "${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system/OS/sys_arch.c"
  )
  set(
    LWIP_SRC 
    ${LWIP_SRC}
    CACHE INTERNAL "LWIP_SRC"
  )
  file(
    GLOB
    STM32CUBEH7_HAL_SRC
    CONFIGURE_DEPENDS
    "${stm32h7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/*.c"
  )
  list(
    REMOVE_ITEM
    STM32CUBEH7_HAL_SRC
    "${stm32h7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_timebase_rtc_alarm_template.c"
    "${stm32h7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_timebase_rtc_wakeup_template.c"
    "${stm32h7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s_ex.c"
  )
  # Remove all low level source files (Low level files include '_ll_' substring)
  set(hal_only "")
  foreach(item ${STM32CUBEH7_HAL_SRC})
    if(NOT ${item} MATCHES "_ll_")
      list(APPEND hal_only ${item})
    endif()
  endforeach()
  set(
    STM32CUBEH7_HAL_SRC
    ${hal_only}
    CACHE INTERNAL "STM32CUBEH7_HAL_SRC"
  )
  message(DEBUG "Exporting include paths: ${STM32CUBEH7_HAL_INCLUDE} ${STM32CUBEH7_CMSIS_INCLUDE} ${STM32CUBEH7_FREERTOS_INCLUDE} ${STM32CUBEH7_BSP_INCLUDE} ${LWIP_INCLUDE}")
endfunction()
