set(EXECUTABLE "test-pwm")
add_executable(
    ${EXECUTABLE} 
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_rcc.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_rcc_ex.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_hsem.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_cortex.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_flash.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_flash_ex.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_pwr.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_pwr_ex.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_gpio.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_i2c.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_i2c_ex.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_tim.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal_tim_ex.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_ll_delayblock.c
    ${STM32CUBEH7_HAL_SRC}/stm32h7xx_hal.c
    startup_stm32h723zgtx.s
    main.c
)
target_include_directories(
    ${EXECUTABLE}
    PRIVATE ${STM32CUBEH7_HAL_INCLUDE}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_options(
    ${EXECUTABLE}
    PRIVATE ${GCC_FLAGS} ${STM32H723_COMPILE_FLAGS}
    PRIVATE -O0 -g
)
target_link_options(
    ${EXECUTABLE}
    PRIVATE ${STM32H723_COMPILE_FLAGS}
    PRIVATE -T${LINKER_SCRIPT} -Wl,--print-memory-usage -Wl,--gc-sections -static -z muldefs -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--start-group -lc -lm -Wl,--end-group
)
set_target_properties(${EXECUTABLE} PROPERTIES SUFFIX ".elf")