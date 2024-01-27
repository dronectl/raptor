add_executable(
    test-bme280
    ${STM32CUBEH7_HAL_SRC}
    ${CMAKE_SOURCE_DIR}/src/drivers/bme280.c
    startup_stm32h723zgtx.s
    main.c
)
target_include_directories(
    test-bme280
    PRIVATE ${STM32CUBEH7_HAL_INCLUDE}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
set_target_properties(test-bme280 PROPERTIES SUFFIX ".elf")
target_compile_options(
    test-bme280
    PRIVATE ${GCC_FLAGS} ${STM32H723_COMPILE_FLAGS}
    PRIVATE -O0 -g
)
target_link_options(
    test-bme280
    PRIVATE ${STM32H723_COMPILE_FLAGS}
    PRIVATE -T${LINKER_SCRIPT} -Wl,--print-memory-usage -Wl,--gc-sections -static -z muldefs -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--start-group -lc -lm -Wl,--end-group
)