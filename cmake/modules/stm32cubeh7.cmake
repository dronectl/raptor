message(STATUS "Building STM32 CubeH7 library")
include(FetchContent)
message(STATUS "Fetching STM32H7 CMSIS HAL and middleware dependancies")
FetchContent_Declare(
  stm32h7
  GIT_REPOSITORY https://github.com/STMicroelectronics/STM32CubeH7.git
  GIT_TAG v1.11.1
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(stm32h7)
set(
  STM32CUBEH7_CMSIS_INCLUDE
  ${stm32h7_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
  ${stm32h7_SOURCE_DIR}/Drivers/CMSIS/Core/Include
  ${stm32h7_SOURCE_DIR}/Drivers/CMSIS/Include
)
set(
  STM32CUBEH7_FREERTOS_INCLUDE
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/
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
)
set(
  STM32CUBEH7_HAL_INCLUDE
  ${stm32h7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
)
set(
  STM32CUBEH7_BSP_INCLUDE
  ${stm32h7_SOURCE_DIR}/Drivers/BSP/Components/lan8742
  ${stm32h7_SOURCE_DIR}/Drivers/BSP/STM32H7xx_Nucleo
)
set(STM32CUBEH7_SRC ${stm32h7_SOURCE_DIR})
set(
  LWIP_INCLUDE
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include
  ${stm32h7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system
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
)
# HAL source to be included in CMakeLists.txt
set(
  STM32CUBEH7_HAL_SRC
  ${stm32h7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src
)

set(STM32CUBEH7_LIB libstm32cubeh7)

add_library(
  ${STM32CUBEH7_LIB}
  STATIC
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_adc.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_adc_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_rcc.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_rcc_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_gpio.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_dma.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_dma_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_mdma.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_hsem.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_wwdg.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_fdcan.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_exti.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_eth_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_eth.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_rtc.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_rtc_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_uart.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_uart_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_cortex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_flash.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_flash_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_pwr.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_pwr_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_spi.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_spi_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_i2c.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_i2c_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_sd.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_sd_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_tim.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_tim_ex.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_ll_sdmmc.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_ll_delayblock.c
  ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal.c
  ${STM32CUBEH7_SRC}/Drivers/BSP/Components/lan8742/lan8742.c
  ${STM32CUBEH7_SRC}/Drivers/BSP/STM32H7xx_Nucleo/stm32h7xx_nucleo.c
  ${LWIP_SRC}
  ${STM32CUBEH7_HAL_SRC}
  ${STM32CUBEH7_FREERTOS_SRC}
)

target_include_directories(${STM32CUBEH7_LIB} PUBLIC include)
target_include_directories(${STM32CUBEH7_LIB} PUBLIC ${LWIP_INCLUDE})
target_include_directories(${STM32CUBEH7_LIB} PUBLIC ${STM32CUBEH7_FREERTOS_INCLUDE})
target_include_directories(${STM32CUBEH7_LIB} PUBLIC ${STM32CUBEH7_HAL_INCLUDE})
target_include_directories(${STM32CUBEH7_LIB} PUBLIC ${STM32CUBEH7_BSP_INCLUDE})
target_include_directories(${STM32CUBEH7_LIB} PUBLIC ${STM32CUBEH7_CMSIS_INCLUDE})

target_compile_options(${STM32CUBEH7_LIB} PUBLIC ${STM32H723_COMPILE_FLAGS})
target_link_options(${STM32CUBEH7_LIB} PUBLIC ${STM32H723_LINKER_FLAGS})

