cmake_minimum_required(VERSION 3.14)
# Check if Git is available
find_program(GIT_EXECUTABLE git)

if(NOT GIT_EXECUTABLE)
  message(FATAL_ERROR "Git not found. Please install Git to proceed.")
else()
  message(STATUS "Found git executable: ${GIT_EXECUTABLE}")
endif()

# Path where you want to clone the library
set(STM32H7_SOURCE_DIR "${CMAKE_BINARY_DIR}/external/STM32CubeH7")

function(clone_repo_if_missing url dir)
  if(NOT EXISTS "${dir}")
    message(STATUS "Cloning ${url} into ${dir}")
    execute_process(
      COMMAND git clone --depth 1 --recurse-submodules ${url} ${dir}
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
      RESULT_VARIABLE GIT_CLONE_RESULT
    )
    if(NOT GIT_CLONE_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to clone repository: ${url}")
    endif()
    else()
      message(STATUS "Repository already cloned: ${dir}")
  endif()
endfunction()

message(STATUS "Fetching STM32H7 CMSIS HAL and middleware dependancies")
clone_repo_if_missing("https://github.com/dronectl/STM32CubeH7.git" "${STM32H7_SOURCE_DIR}")

message(STATUS "Exporting FreeRTOS Src and Includes")
set(
  CMSIS_INCLUDE
  ${STM32H7_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
  ${STM32H7_SOURCE_DIR}/Drivers/CMSIS/Include
  ${STM32H7_SOURCE_DIR}/Drivers/CMSIS/Core/Include
  CACHE INTERNAL "CMSIS_INCLUDE"
)

set(
  FREERTOS_SRCS
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/croutine.c
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/list.c
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/queue.c
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/tasks.c
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/timers.c
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
  CACHE INTERNAL "FREERTOS_SRCS"
)
set(
  FREERTOS_INC
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/
  ${CMSIS_INCLUDE}
  CACHE INTERNAL "FREERTOS_INC"
)

message(STATUS "Exporting LWIP Src and Includes")
file(
  GLOB_RECURSE 
  LWIP_SRCS_TMP
  CONFIGURE_DEPENDS
  "${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/core/*.c"
  "${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/api/*.c"
  "${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/netif/ethernet.c"
  "${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system/OS/sys_arch.c"
)

set(
  LWIP_SRCS
  ${LWIP_SRCS_TMP}
  CACHE INTERNAL "LWIP_SRCS"
)
set(
  LWIP_INC
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include
  ${STM32H7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system
  CACHE INTERNAL "LWIP_INC"
)

message(STATUS "Exporting HAL Src and Includes")
aux_source_directory(${STM32H7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src HAL_SRCS_TMP)
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
  ${STM32H7_SOURCE_DIR}/Drivers/BSP/Components/lan8742/lan8742.c
  CACHE INTERNAL "HAL_SRCS"
)
set(
  HAL_INC
  ${STM32H7_SOURCE_DIR}/Drivers/BSP/Components/lan8742
  ${STM32H7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
  ${CMSIS_INCLUDE}
  CACHE INTERNAL "HAL_INC"
)

