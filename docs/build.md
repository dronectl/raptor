# Firmware Build Configurations

Modified: 2024-01

There are two build configurations:
 1. Debug - Debugging configuration for development
 2. Release - Release configuration for production

## Windows
This cmake build system uses `make` as the make program. To install `make` on windows you can install via `choco`:
```
choco install make
```

On Windows the default CMake make program is `nmake`. To override the default you will need to specify a generator to cmake when you configure the project. This is done with the `-G` option:
```shell
cmake .. -G "MinGW Makefiles"
```

## Debug
The Debug configurations adds the following compile flags: `-DRAPTOR_DEBUG -O0 -g`. This sets the `RAPTOR_DEBUG` preprocessor macro to toggle the visibility of certain blocks of code. For example we would disable the blocks of code which enable and service the watchdog subsystem while debugging:
```c
    ...
    case HEALTH_SERVICE:
#ifndef RAPTOR_DEBUG
      service_watchdog();
#endif // RAPTOR_DEBUG
      next_state = HEALTH_READ;
      break;
    ...
```

We also turn off optimizations (`-O0`) and enable debug symbols (`-g`) for `gdb`.

Configuring cmake for debug simply involves passing `Debug` to the `CMAKE_BUILD_TYPE` option:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## Release
The Release configuration simply adds `-Os` optimization.

Configuring cmake for release simply involves passing `Release` to the `CMAKE_BUILD_TYPE` option:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```
