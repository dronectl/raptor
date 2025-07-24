#
# CubeH7.cmake
#
# Fetch CubeH7 sources and build static libraries
#

cmake_minimum_required(VERSION 3.14)


function(cubeh7_fetch)
    set(CUBEH7_SOURCE_DIR "${CMAKE_BINARY_DIR}/external/STM32CubeH7" CACHE INTERNAL "CUBEH7 Source directory")
    if(NOT EXISTS "${CUBEH7_SOURCE_DIR}")
        set(CUBEH7_URL "https://github.com/dronectl/STM32CubeH7.git")
        message(STATUS "Cloning ${CUBEH7} into ${CUBEH7_SOURCE_DIR}")
        execute_process(
            COMMAND git clone --depth 1 --recurse-submodules ${CUBEH7_URL} ${CUBEH7_SOURCE_DIR}
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            RESULT_VARIABLE GIT_CLONE_RESULT
        )
        if(NOT GIT_CLONE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to clone repository: ${CUBEH7_URL}")
        endif()
    endif()
endfunction()

function(cubeh7_add_lwip CONFIG_DIR)
    cubeh7_fetch()
    add_library(lwip STATIC)
    set_target_properties(lwip PROPERTIES PREFIX "")
    file(
        GLOB_RECURSE 
        LWIP_SRCS_TMP
        CONFIGURE_DEPENDS
        "${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/core/*.c"
        "${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/api/*.c"
        "${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/netif/ethernet.c"
        "${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system/OS/sys_arch.c"
    )
    set(LWIP_SRCS ${LWIP_SRCS_TMP})
    target_sources(lwip PRIVATE ${LWIP_SRCS})
    target_include_directories(
        lwip
        PUBLIC
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/LwIP/system
        ${CONFIG_DIR}
    )
endfunction()

function(cubeh7_add_cmsis TARGET)
    cubeh7_fetch()
    target_include_directories(
        ${TARGET}
        PUBLIC
        ${CUBEH7_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
        ${CUBEH7_SOURCE_DIR}/Drivers/CMSIS/Include
        ${CUBEH7_SOURCE_DIR}/Drivers/CMSIS/Core/Include
    )
endfunction()

function(cubeh7_add_freertos)
    cubeh7_fetch()
    add_library(
        freertos
        STATIC
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/croutine.c
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/list.c
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/queue.c
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/tasks.c
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/timers.c
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    )
    target_include_directories(
        freertos
        PUBLIC
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
        ${CUBEH7_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/
        ${CMSIS_INCLUDE}
    )
endfunction()

function(cubeh7_add_hal)
    cubeh7_fetch()
    add_library(hal STATIC)
    aux_source_directory(${CUBEH7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src HAL_SRCS_TMP)
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
        ${CUBEH7_SOURCE_DIR}/Drivers/BSP/Components/lan8742/lan8742.c
    )
    target_sources(hal PRIVATE ${HAL_SRCS})
    target_include_directories(
        hal
        PUBLIC
        ${CUBEH7_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
        ${CUBEH7_SOURCE_DIR}/Drivers/BSP/Components/lan8742
        ${CMSIS_INCLUDE}
    )
endfunction()

