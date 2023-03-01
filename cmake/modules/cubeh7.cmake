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
    PARENT_SCOPE
  )
  set(
    STM32CUBEH7_HAL_INCLUDE
    ${STM32CUBEH7_CMSIS_INCLUDE}
    ${stm32h7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
    PARENT_SCOPE
  )
  set(
    STM32CUBEH7_BSP_INCLUDE
    ${stm32h7_SOURCE_DIR}/Drivers/BSP/Components/lan8742
    ${stm32h7_SOURCE_DIR}/Drivers/BSP/STM32H7xx_Nucleo
    PARENT_SCOPE
  )
  set(STM32CUBEH7_SRC ${stm32h7_SOURCE_DIR} PARENT_SCOPE)
  set(
    LWIP_INCLUDE
    ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include
    ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system
    PARENT_SCOPE
  )
  file(
    GLOB_RECURSE 
    LWIP_SRC
    CONFIGURE_DEPENDS
    "${stm32h7_SOURCE_DIR}//Middlewares/Third_Party/LwIP/src/core/*.c"
    "${stm32h7_SOURCE_DIR}//Middlewares/Third_Party/LwIP/src/api/*.c"
    "${stm32h7_SOURCE_DIR}//Middlewares/Third_Party/LwIP/src/netif/*.c"
  )
  set(
    LWIP_SRC 
    ${LWIP_SRC}
    PARENT_SCOPE
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
  set(
    STM32CUBEH7_HAL_SRC 
    ${STM32CUBEH7_HAL_SRC}
    PARENT_SCOPE
  )
  message(DEBUG "Exporting include paths: ${STM32CUBEH7_CMSIS_INCLUDE} ${STM32CUBEH7_BSP_INCLUDE} ${LWIP_INCLUDE}")
endfunction()
