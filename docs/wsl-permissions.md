# USB Permissions in WSL

In order for the Cortex-Debugger extension to work in WSL, we need to have access to the USB plugin for STLink. In order to do this we need to define a `udev` rule in WSL. Create a new file `/etc/udev/rules.d/60-openocd.rules60-openocd.rules` and add the following line:
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