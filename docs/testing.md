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

## Debugging

Ensure the board is flashed with a binary (compiled with `-g`) and is connected via USB to your host machine.

### GDB and OpenOCD
Open a `gdbserver` session with `openocd`:
```bash
openocd -futils/stm32h723.cfg
```

In another terminal start `gdb` from the project root and pass the binary path as an argument. This will source the `~/.gdbinit` on startup:
```bash
christiansargusingh  22:07:08 Projects/dronectl/raptor % arm-none-eabi-gdb build/core/raptor.elf
```

You may get an auto-load warning:
```bash
warning: File "/Users/christiansargusingh/Projects/dronectl/raptor/.gdbinit" auto-loading has been declined by your `auto-load safe-path' set to "$debugdir:$datadir/auto-load".
To enable execution of this file add
        add-auto-load-safe-path /Users/christiansargusingh/Projects/dronectl/raptor/.gdbinit
line to your configuration file "/Users/christiansargusingh/Library/Preferences/gdb/gdbinit".
To completely disable this security protection add
        set auto-load safe-path /
line to your configuration file "/Users/christiansargusingh/Library/Preferences/gdb/gdbinit".
For more information about this security protection see the
"Auto-loading safe path" section in the GDB manual.  E.g., run from the shell:
        info "(gdb)Auto-loading safe path"
```

If so follow the instructions to add the path to the repository root as an auto-load safe path.

### Debug Adapter Protocol (VSCode)
To debug test binaries you will need to select the `Test Debug (OpenOCD)` launch configuration and change the `executable` field to point to the correct test binary. For example if testing the bme280 binary:
```json
{
    ...
    "executable": "${workspaceRoot}/build/tests/bme280/test.elf",
    ...
}
```

> I am looking for a way to specify the folder as a prompt in vscode. If anyone knows how to achieve this feel free to submit a PR.

## Unittesting

Configure the unittest project.
```bash
mkdir tests/build
cmake -B tests/build -S tests/unittests -DCMAKE_BUILD_TYPE=Debug
make -C tests/build -j
```

Run the unittest binary
```bash
./tests/build/raptor-test
```

Run the coverage report powered by `gcovr`:
```bash
make -C tests/build coverage
```
You should get a report generated in stdout and an accompanying xml file named `coverage.xml`:
```bash
------------------------------------------------------------------------------
                           GCC Code Coverage Report
Directory: /Users/christiansargusingh/Projects/dronectl/raptor
------------------------------------------------------------------------------
File                                    Branches   Taken  Cover   Missing
------------------------------------------------------------------------------
core/lib/cbuffer.c                            14       9    64%   26,35,46,47
core/lib/logger.c                             16       1     6%   49,74,85,91,97,103,104,146,154,166,171
core/lib/scpi/commands.c                      16       0     0%   68,70,77,80,87,91,96,102,106,111,117,118,119,122
core/lib/scpi/err.c                            6       5    83%   42
core/lib/scpi/lexer.c                         18       0     0%   37,39,42,49,56,76,109,111,126
core/lib/scpi/parser.c                        28       0     0%   31,33,39,44,54,58,63,73,79,87,94,99,103,104,113,128
core/lib/scpi/utf8.c                          20      16    80%   81,89,97,105
core/lib/sysreg.c                             94      57    60%   201,204,211,214,223,225,226,229,230,233,234,237,238,254,274,301,305,315,319,323,325,335,339,349,353,357,359,369,373,383,387,391,393,403,407,417,421
------------------------------------------------------------------------------
TOTAL                                        212      88    41%
------------------------------------------------------------------------------
Cobertura code coverage report saved in coverage.xml.
[100%] Built target coverage
```

### Note on LLVM Clang
For MacOS users, you must install and use a gcc toolchain in order to build the unittests. The firmware contains some gcc specific attributes and directives which will cause compile errors on LLVM based compilers such as clang. An example of this is using the section attribute:
```c
__attribute__((section(".ram_d3"))) static StaticTask_t scpi_task_buffer;
```
To avoid clutter in the firmware for supporting multiple compilers I opted to restrict the scope of the unittests to just gcc based compilers.

To compile and run the unittests with coverage you need to point cmake to use the correct c and cxx compilers. For example, if my gcc compilers are located in `/opt/homebrew/bin` I can configure the cmake project using the following cmake options:
```bash
-DCMAKE_C_COMPILER=/opt/homebrew/bin/gcc-14 -DCMAKE_CXX_COMPILER=/opt/homebrew/bin/g++-14
```

Coverage will also not work unless you also specify the GCOV binary. This is because by default on MacOS, the `gcov` executable points to an llvm-based gcov which will not work for coverage symbols generated by the gcc linker. To solve this I have provided the optional `RAPTOR_COVERAGE_GCOV` variable for pointing to the gcc-based `gcov` binary. For example if my gcc-based gcov is discoverable by invoking `gcov-14` I can add the following to the cmake configure step:
```bash
-DRAPTOR_COVERAGE_GCOV=gcov-14
```
