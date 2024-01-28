# CubeIDE/MX Integration

Modified: 2024-01

The purpose of this folder is to provide:
- [x] Firmware debugging using CubeIDE.
- [x] Source controlled MCU hardware configuration (`.ioc`) using CubeMX.
- [x] Sandbox environment for code generation (particularly for peripheral configuration).

The contents of this directory are outside the scope of the main project. This is for the following reasons:
1. Sterility of the main build environment which uses custom CMake configuration.
2. This project can be imported directly into CubeIDE to in order to: 
    - edit hardware configuration
    - generate code and
    - debug the source firmware.  

For details on how to use this directory see the [developers guide](/docs/dev.md).