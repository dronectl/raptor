cmake_minimum_required(VERSION 3.22)

function(fetch_cmsis)
  include(FetchContent)
  message(STATUS "Fetching STM32H7 CMSIS and HAL dependancies")
  FetchContent_Declare(
    stm32h7
    GIT_REPOSITORY https://github.com/STMicroelectronics/STM32CubeH7.git
    GIT_TAG v1.11.0
  )
  FetchContent_MakeAvailable(stm32h7)
  # set stm32h7 include paths
  set(
    STM32CUBEH7_CMSIS_INCLUDE
    ${stm32h7_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${stm32h7_SOURCE_DIR}/Drivers/CMSIS/Core/Include
    PARENT_SCOPE
  )
  set(STM32CUBEH7_CMSIS_SRC ${stm32h7_SOURCE_DIR} PARENT_SCOPE)
  message(DEBUG "Exporting include paths: ${STM32CUBEH7_CMSIS_INCLUDE}")
endfunction()
