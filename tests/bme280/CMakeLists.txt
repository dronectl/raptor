add_executable(
    test 
    ${STM32CUBEH7_HAL_SRC}
    ${CMAKE_SOURCE_DIR}/src/drivers/bme280.c
    startup_stm32h723zgtx.s
    main.c
)
target_include_directories(
    test
    PRIVATE ${STM32CUBEH7_HAL_INCLUDE}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_options(
    test
    PRIVATE ${GCC_FLAGS} ${STM32H723_COMPILE_FLAGS}
    PRIVATE -O0 -g
)
target_link_options(
    test
    PRIVATE ${STM32H723_COMPILE_FLAGS}
    PRIVATE -T${LINKER_SCRIPT} -Wl,--print-memory-usage -Wl,--gc-sections -static -z muldefs -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--start-group -lc -lm -Wl,--end-group
)
set_target_properties(test PROPERTIES SUFFIX ".elf")