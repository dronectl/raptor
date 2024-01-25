set(DEVICE_SUPPORT_ROOT "${C2000WARE_ROOT}/device_support/f28004x")
set(DRIVERLIB_ROOT "${C2000WARE_ROOT}/driverlib/f28004x/driverlib")

# ###########
# BME280 Test
# ###########

add_executable(
    test_bme280
    ${STM32CUBEH7_HAL_SRC}
    test_bme280.c
)
set_target_properties(test_bme280 PROPERTIES SUFFIX ".elf")

target_include_directories(
    test_bme280
    PRIVATE ${STM32CUBEH7_HAL_SRC}
)
target_include_directories(${EXECUTABLE} PRIVATE ${STM32CUBEH7_HAL_INCLUDE})
target_include_directories(${EXECUTABLE} PRIVATE ${STM32CUBEH7_BSP_INCLUDE})
target_include_directories(${EXECUTABLE} PRIVATE include)
target_include_directories(${EXECUTABLE} PRIVATE ${STM32CUBEH7_CMSIS_INCLUDE})
set_target_properties(${EXECUTABLE} PROPERTIES OUTPUT_NAME ${EXECUTABLE}.elf)

target_include_directories(
    test_bme280
    PUBLIC include
    PUBLIC ${DEVICE_SUPPORT_ROOT}/headers/include
    PUBLIC ${DEVICE_SUPPORT_ROOT}/common/include
    PUBLIC ${DRIVERLIB_ROOT}
    PUBLIC ${TOOLCHAIN_ROOT}/include
    PUBLIC ${TOOLCHAIN_ROOT}/include/stlport
)
target_link_directories(
    test_bme280
    PUBLIC include
    PUBLIC ${TOOLCHAIN_ROOT}/lib
    PUBLIC ${DEVICE_SUPPORT_ROOT}/headers/cmd
    PUBLIC ${DEVICE_SUPPORT_ROOT}/common/cmd
)

target_link_libraries(
    test_bme280
    28004x_generic_flash_lnk.cmd
    -llibc.a
    ${DRIVERLIB_ROOT}/ccs/Debug/driverlib.lib
)
