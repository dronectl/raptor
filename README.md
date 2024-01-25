# Raptor Firmware
[![ci](https://github.com/dronectl/raptor/actions/workflows/ci.yaml/badge.svg)](https://github.com/dronectl/raptor/actions/workflows/ci.yaml)

Modified: 2023-01

Firmware for *Raptor*, an integrated HIL test platform for UAV propulsion systems.

## Navigation
1. [Features](#features)
2. [Contributing](#contributing)
3. [Issues](#issues)
4. [License](#license)

## Features
 - Streaming over Ethernet
    - BLDC Motor thrust sampling @100ksps
    - BLDC power sampling @100ksps
    - Vibration frequencies @100ksps
 - Alive data for monitoring low frequency measurements:
    - Relative Humidity
    - Pressure
    - Ambient Temperature
    - ESC Temperature
    - BLDC Motor Temperature
 - Programmable hardware tripzones for automatic shutoff



## Contributing
For details on getting started as a contributor visit the [development documentation](/docs/dev.md).

## Issues
For any questions or inquiries please contact christian911@sympatico.ca or open an issue and tag it with `triage`.

## License
This project is licensed under the terms of the [GNU GPLv3](LICENSE) license.