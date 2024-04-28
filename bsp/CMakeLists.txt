cmake_minimum_required(VERSION 3.14)

include(stm32cubeh7)

set(BSP_SRCS "")
aux_source_directory(src BSP_SRCS)
list(APPEND BSP_SRCS raptor/startup_stm32h723xx.s)
set(BSP_INC inc)

# point to stubs in unittest environment
if (RAPTOR_TESTS)
  aux_source_directory(BSP_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unittest/mocks/bsp")
  aux_source_directory(HAL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unittest/mocks/hal")
  set(LIB_BSP_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/tests/unittest/mocks/bsp")
  set(HAL_INC "${CMAKE_CURRENT_SOURCE_DIR}/tests/unittest/mocks/hal")
  set(STM32H723_COMPILE_FLAGS "")
  set(STM32H723_LINKER_FLAGS "")
endif()

# defined as an object library for proper weak symbol overrides
add_library(${LIB_BSP} OBJECT ${BSP_SRCS} ${HAL_SRCS})
target_include_directories(${LIB_BSP} PUBLIC ${HAL_INC} ${BSP_INC})
target_compile_options(${LIB_BSP} PUBLIC ${STM32H723_COMPILE_FLAGS})
target_link_options(${LIB_BSP} PUBLIC ${STM32H723_LINKER_FLAGS})

