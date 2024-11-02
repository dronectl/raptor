cmake_minimum_required(VERSION 3.22)

set(CMAKE_STATIC_LIBRARY_PREFIX "")
add_library(
  ${LIB_MOCK}
  queue.c
  FreeRTOS.c
  task.c
  lwip/sockets.c
  lwip/inet.c
)
target_include_directories(${LIB_MOCK} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(${LIB_MOCK} PUBLIC -g -O0)

