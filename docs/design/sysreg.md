# System Registers

Modified: 2024-03

Access to each register is performed using the `get` `set` style functions scoped by type. For example to get the device UUID register (2 byte):
```c
uint16_t uuid;
sysreg_get_u16(SYSREG_UUID, &uuid);
printf("UUID: %u", uuid);
```

Register sets operate the same way except on a `const` pointer:
```c
const uint16_t uuid = 456;
sysreg_set_u16(SYSREG_UUID, &uuid);
```

Registers access is also optionally mutable during runtime to 'lock' registers while running critical tasks:
```c
sysreg_set_access(SYSREG_UUID, SYSREG_ACCESS_W);
```

> Note: registers can only be modified if the lock bit of the access control register is unset. The lockbit cannot be modified at runtime and is defined statically as part of the register configuration.

Each register has a configuration struct with a set of properties defining:
1. Primitive Type (`uint8_t`, `float`, etc...)
2. Reset value (default)
3. Minimum Value
4. Maximum Value
5. Access Permissions (L, RW, RO, WO)

The register configurations are implemented as a lookup table and mostly consist of constants with the exception of access permissions control.

The system registers are statically defined to limit exposure and enforce sanitization procedures. To ensure the system register values are always valid, register set operations execute the following sanitization procedures before writing an input value to a register:
1. Verify register offset
2. Verify write access permission and perform type check
3. Saturate input to min and max range
4. Write to register


