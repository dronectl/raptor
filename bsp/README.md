# Board Support Package (BSP)

Modified: 2024-04

The board support package provides:

 1. Auto-generated hardare specific code
 2. CubeMX hardware pinmux file (`.ioc`) -> used to generate BSP code.
 3. Firmware debugging suppport for CubeIDE

The code generated in this folder is source controlled as the project BSP which is derived from the CubeMX `.ioc` file which describes the pinouts for the MCU. This ensures that the BSP and hardware configuration is in lockstep for each release.

The BSP is used by our CMake configuration to build a portable static library for linking against application firmware and for experimental integration work.

For more details on how to use this directory see the [developers guide](/docs/dev.md).

## Notes

### Adding and Deleting BSP Files
To limit the requirement for developers to modify the CMake build configuration, I opted to use CMake's `aux_source_directory` function targeting the `src` directory to autoload all source files. This has a caveat in which CMake is unable to detect if files are added or removed after the CMake project has been configured since the function only executes once during project configuration and pushes the results to a variable. The workaround is to rerun the `cmake` project configuration step to force CMake to reload the source storage variable.

