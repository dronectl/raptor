add_executable(
    test-hx711 
    ${STM32CUBEH7_HAL_SRC}
    ${CMAKE_SOURCE_DIR}/src/drivers/hx711.c
    ${STM32CUBEH7_SRC}/Drivers/BSP/STM32H7xx_Nucleo/stm32h7xx_nucleo.c
    startup_stm32h723zgtx.s
    main.c
)
target_include_directories(
    test-hx711
    PRIVATE ${STM32CUBEH7_HAL_INCLUDE}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${STM32CUBEH7_BSP_INCLUDE}
)
target_compile_options(
    test-hx711
    PRIVATE ${GCC_FLAGS} ${STM32H723_COMPILE_FLAGS}
    PRIVATE -O0 -g
)
target_link_options(
    test-hx711
    PRIVATE ${STM32H723_COMPILE_FLAGS}
    PRIVATE -T${LINKER_SCRIPT} -Wl,--print-memory-usage -Wl,--gc-sections -static -z muldefs -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--start-group -lc -lm -Wl,--end-group
)
set_target_properties(test-hx711 PROPERTIES SUFFIX ".elf")
