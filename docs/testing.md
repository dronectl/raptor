# Test Environments

Modified: 2024-01

> Note: This testing setup is experimental and subject to change. If you have any suggestions for improvements to the testing environments please create an issue.

## Brief
In order to test a combination of hardware, STM32 configuration and C modules outside the context of an RTOS, it is recommended to build a sandbox environment for developing a working demo.

Most developers will start from a STM32 Cube project and develop experimental firmware from there. The issue with this approach is these projects contain too much overhead and do not integrate well into our pre-existing toolchain. The goal is to reduce complexity by condensing the surrounding code configuration to a few critical files and integrate it into our cmake build process. This effectively involves building a small bootstrap framework for configuration heavy libraries like the STM32 HAL Library.

This repository contains a `tests` cmake subdirectory which hosts all the raptor test environments. Each folder in the `tests` directory hosts the build configuration, test code and the minimum bootstrap code required to enable its operation. These binaries can be flashed to the board and debugged directly making it easy to:
 1. Test C modules directly from the raptor source firmware
 2. Develop experimental source code in a well encapsulated sandbox environment

## Creating a Test Environment
To create a new test environment create a new folder under `tests`. You can copy the required files and configuration from [bme280](/tests/bme280) and make any neccessary modifications.

Edit `main.c` and `stm32h7xx_hal_conf.h` to configure the required peripherals and HAL modules. Add your test code to `main.c:main`. Here you can write your experimental code in the same `main.c` file or in another local source file or reference a source file in the raptor firmware. 

In any case ensure the correct source and header files are specified in the CMakeLists.txt file. In the cmake code snippets I will use the `bme280` test folder as reference. Note you will likely require `STM32CUBEH7_HAL_SRC` and `STM32CUBEH7_HAL_INCLUDE` for any code using the HAL library:
```cmake
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
```

Include any compile and linker flags for your test binary. You will most likely require `$GCC_FLAGS` `$STM32H723_COMPILE_FLAGS` and `$LINKER_SCRIPT` as defined by the parent scope:
```cmake
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
```

Finally add the new subdirectory to the `tests` CMakeLists.txt:
```cmake
enable_testing()
# Include and add each test subdirectory
...
add_subdirectory(bme280)
```

## Building Test Binaries
Test binaries under the `tests` subdirectory can be built using the `BUILD_TESTS` option. Note that the cmake project must be reconfigured:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=On
```

The tests will now be compiled alongside the raptor source code:
```bash
make -j
```

## Debugging Test Binaries
To debug test binaries you will need to select the `Test Debug (OpenOCD)` launch configuration and change the `executable` field to point to the correct test binary. For example if testing the bme280 binary:
```json
{
    ...
    "executable": "${workspaceRoot}/build/tests/bme280/test.elf",
    ...
}
```

> I am looking for a way to specify the folder as a prompt in vscode. If anyone knows how to achieve this feel free to submit a PR.