## Winwing plugin for X-Plane 12

For X-Plane 12 only.

This plugin allows you to use Winwing devices in X-Plane 12 on MacOS and Linux.
The plugin theoretically works on Windows, but is not required there, as Winwing provides a native Windows driver through the SimAppPro software.

### Installation

1. Download the latest release from the releases page.
2. Unzip the downloaded file.
3. Copy the `Winwing` folder to your X-Plane 12 `Resources/plugins` directory.
4. Start X-Plane 12.

### Usage

- The plugin will automatically detect your Winwing devices.
- There are no user-configurable settings at this time.

### Winwing Devices Compatibility

The table below lists the compatibility status of various Winwing devices with the plugin.

| Device Name                   | Compatibility         | Tested | Roadmap / suggestions                     |
| ----------------------------- | --------------------- | ------ | ----------------------------------------- |
| URSA MINOR Airline Joystick L | 🟢 Partly implemented | 🟢 Yes | Implement vibrations                      |
| URSA MINOR Airline Joystick R | 🟠 Partly implemented | 🔴 No  | Implement vibrations, find the product ID |
| MCDU-32                       | 🟢 Fully implemented  | 🟢 Yes |                                           |
| FCU Unit                      | 🔴 Not implemented    | 🔴 No  |                                           |
| EFIS R                        | 🔴 Not implemented    | 🔴 No  |                                           |
| EFIS L                        | 🔴 Not implemented    | 🔴 No  |                                           |
| Panel of Autopilot 3          | 🔴 Not implemented    | 🔴 No  |                                           |
| PFP 4                         | 🟠 Partly implemented | 🔴 No  |                                           |
| PFP 7                         | 🟠 Partly implemented | 🔴 No  |                                           |
| PFP 3N                        | 🟠 Partly implemented | 🔴 No  |                                           |

### Aircraft Compatibility

Below is a table of aircraft that are compatible with the plugin.

| Aircraft     | Compatibility      | Tested |
| ------------ | ------------------ | ------ |
| Toliss A319  | Fully implemented  | 🟢 Yes |
| Toliss A320  | Fully implemented  | 🟢 Yes |
| Toliss A321  | Fully implemented  | 🔴 No  |
| Zibo 737-800 | Partly implemented | 🔴 No  |

### Credits

- [@Schenlap](https://github.com/schenlap) for the initial python implementation and providing the HID protocol documentation.
- [@zodiac1214](https://github.com/zodiac1214) for the Ursa Minor Joystick HID protocol.
