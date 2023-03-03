cmake_minimum_required(VERSION 3.22)

# Fetch the FreeRTOS Kernel and pass configuration for build
function(fetch_freertos_kernel HEAP PORT CONFIG_FILE_DIRECTORY)
  include(FetchContent)
  # Include the FreeRTOS source code
  message(STATUS "Fetching FreeRTOS kernel ...")
  set(FREERTOS_CONFIG_FILE_DIRECTORY ${CONFIG_FILE_DIRECTORY} CACHE STRING "")
  FetchContent_Declare(
    freertos_kernel
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
    GIT_TAG V10.5.1
  )
  add_library(freertos_config INTERFACE)
  target_include_directories(freertos_config SYSTEM INTERFACE include)
  target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)
  set(FREERTOS_HEAP "${HEAP}" CACHE STRING "" FORCE)
  # Select the native compile PORT
  set(FREERTOS_PORT "${PORT}" CACHE STRING "" FORCE)
  FetchContent_MakeAvailable(freertos_kernel)
endfunction()