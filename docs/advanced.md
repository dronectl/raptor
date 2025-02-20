# Advanced Troubleshooting

This document describes some more advanced debugging utilities which are useful for more involved problems.

## Protobuf UDP Dissectors in Wireshark
We can configure wireshark (version 3.2.0 and later) to decode UDP packets by passing it the root protobuf package path.

![alt text](/docs/img/wireshark-protobuf.png)

Open wireshark and navigate to `Preferences > Protocols > ProtoBuf` then select the following options:

![alt text](/docs/img/wireshark-pb-settings.png)

Select `Edit` on the `Protobuf Search Paths` and configure the path to point to the `protobuf` package root in the `protocols` submodule (or in the base repository):

![alt text](/docs/img/wireshark-pb-search-paths.png)

Select `Edit` on the `Protobuf UDP message types` and configure the uhci base message and response types on the UHCI UDP discovery port (for example):

![alt text](/docs/img/wireshark-pb-msg.png)

 > We also need a TCP dissector!

## Cortex M7 Hard Fault Diagnostics
We can determine the root cause of a hardfault by inspecting the `SCB->CFSR_UFSR_BFSR_MMFSR` register. For example, an address misalignment in LWIP caused an intermittent hard fault in the past. This could be identified by unaligned bit being set.

![alt text](/docs/img/cortex-hardfault-diag.png)

## Cortex Debugger in WSL

If I try to run a debug instance in VSCode from my Ubuntu 22.04 WSL2 environment I get the following error:
```bash
...
Error: libusb_open failed with LIBUSB_ERROR_NOT_FOUND
Error: open failed
```

In order for the `cortex-debugger` extension to work in WSL, we need to have the correct access permissions to the USB plugin for STLink. In order to do this we need to define a `udev` rule in WSL. Create a new file `/etc/udev/rules.d/60-openocd.rules60-openocd.rules` and add the following line:
```
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374e", MODE="666", GROUP="plugdev", TAG+="uaccess"
```

> Note the `idVendor` and `idProduct` are specific to the STLink used. To find this information you can use usbipd.

Ensure our user is part of the `plugdev` group:
```bash
sudo usermod -aG plugdev $USER
```

Reload udev to apply the new rules:
```bash
sudo udevadm control --reload
```

Unplug STLink and replug. Apply usbipd bridge:

```powershell
PS C:\Windows\system32> usbipd wsl list
BUSID  VID:PID    DEVICE                                                        STATE
3-8    8087:0025  Intel(R) Wireless Bluetooth(R)                                Not attached
6-4    0483:374e  ST-Link Debug, USB Mass Storage Device, STMicroelectronic...  Not attached

PS C:\Windows\system32> usbipd wsl attach --busid=6-4
```

Check in WSL that we have access to the STLink:
```bash
cs@titan:~/projects/dronectl/raptor/build$ lsusb
Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
Bus 001 Device 004: ID 0483:374e STMicroelectronics STLINK-V3
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
```

Now we can run OpenOCD without root permissions.

## Resources
https://community.st.com/t5/stm32-mcus/how-to-debug-a-hardfault-on-an-arm-cortex-m-stm32/ta-p/672235