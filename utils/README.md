# Firmware Utilities

Modified: 2024-01

This folder contains the a mix of both development tool and firmware configuration.

> Please do not modify these files unless you are confident you know what you are doing. Making improper changes to these files can cause widespread development issues.

Here is a brief overview of each file and its purpose:

 - [c_cpp_properties.json](vscode/c_cpp_properties.json) - VSCode project code editing configuration allows intellisense and other editor features.
 - [launch.json](vscode/launch.json) - VSCode debugging configuration enables you to debug directly in the VSCode editor using the `cortex-debug` extension.
 - [stm32h7x3.svd](stm32h7x3.svd) Maps the MCU peripherals to memory addresses so the VSCode `cortex-debug` extension can monitor peripheral registers.
 - [stm32h723.cfg](stm32h723.cfg) - OpenOCD configuration to enable firmware flashing using STLINK and debugging using GDB.
 - [STM32H723ZGTX_FLASH.ld](STM32H723ZGTX_FLASH.ld) - Linker script for flash loaded code execution (default).
 - [STM32H723ZGTX_RAM.ld](STM32H723ZGTX_RAM.ld) - Linker script for RAM loaded code (not recommended for production).

For more information on using these files for development refer to the [developers guide](/docs/dev.md).