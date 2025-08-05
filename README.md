## Winwing plugin for X-Plane 12

For X-Plane 12 only.

This plugin allows you to use Winwing devices in X-Plane 12 on MacOS and Linux.
The plugin theoretically works on Windows, but is not required there, as Winwing provides a native Windows driver through the SimAppPro software.

### Installation

1. Download the latest release from the releases page.
2. Unzip the downloaded file.
3. Copy the `Winwing` folder to your X-Plane 12 `Resources/plugins` directory.
4. Start X-Plane 12.
5. Updating can be done by replacing the `Winwing` folder or using Skunkcrafts Updater if you have it installed.

### Usage

- The plugin will automatically detect your Winwing devices.
- There are no user-configurable settings at this time.

### Compatibility Matrix

The matrix below shows device and aircraft compatibility. Devices are listed vertically, aircraft horizontally.

| Device                    | Status | Toliss A3xx | Laminar A330 | AeroGenesis A330 | Zibo 737 | IXEG 737 | FF 777 | SSG 747 | Notes                                         |
| ------------------------- | ------ | ----------- | ------------ | ---------------- | -------- | -------- | ------ | ------- | --------------------------------------------- |
| **URSA MINOR Joystick L** | 🟢     | 🟢          | 🟢           | 🟢               | 🟢       | 🟢       | 🟢     | 🟢      | Fully implemented                             |
| **URSA MINOR Joystick R** | 🟠     | 🔴          | 🔴           | 🔴               | 🔴       | 🔴       | 🔴     | 🔴      | Missing USB product ID - need device owner    |
| **MCDU-32**               | 🟢     | 🟢          | 🟢           | 🟢               | 🔴       | 🔴       | 🔴     | 🔴      | FF777: Need FMC button info, SSG: Dual FMC    |
| **PFP 3N**                | 🟢     | 🔴          | 🔴           | 🔴               | 🟢       | 🟢       | 🟢     | 🟢      | Fully implemented                             |
| **PFP 4**                 | 🟢     | 🔴          | 🔴           | 🔴               | 🟢       | 🟢       | 🟢     | 🟢      | Fully implemented                             |
| **PFP 7**                 | 🟢     | 🔴          | 🔴           | 🔴               | 🟢       | 🟢       | 🟢     | 🟢      | Fully implemented                             |
| **Panel of Autopilot 3**  | 🔴     | 🔴          | 🔴           | 🔴               | 🔴       | 🔴       | 🔴     | 🔴      | Not implemented                               |
| **FCU-EFIS (FCU Unit)**   | 🟢     | 🟢          | 🔴           | 🔴               | 🔴       | 🔴       | 🔴     | 🔴      | Fully implemented for Toliss                  |
| **FCU-EFIS (EFIS R)**     | 🟠     | 🟢          | 🔴           | 🔴               | 🔴       | 🔴       | 🔴     | 🔴      | Fully implemented for Toliss - but not tested |
| **FCU-EFIS (EFIS L)**     | 🟢     | 🟢          | 🔴           | 🔴               | 🔴       | 🔴       | 🔴     | 🔴      | Fully implemented for Toliss                  |

#### Legend

- 🟢 **Fully implemented** - All features working
- 🟠 **Partly implemented** - Some limitations or missing features
- 🔴 **Not implemented** - No support

### Known Issues

- It seems that upon startup, the MCDU reverts to the default font, even if you used SimAppPro to change it. This is due to the startup message "waking" up the MCDU, which resets the font. More research is needed on the startup sequence of the MCDU to find a solution.

### Credits

- [@Schenlap](https://github.com/schenlap) for the initial python implementation and providing the HID protocol documentation.
- [@zodiac1214](https://github.com/zodiac1214) for the Ursa Minor Joystick HID protocol.

### Demonstration

<img src="https://github.com/user-attachments/assets/75d4e3e0-af9e-488f-bd5e-2d834bea110d" alt="Airbus A20N" width="256" />
<img src="https://github.com/user-attachments/assets/8f5750e2-f913-479a-9f7a-6e3d6c31382d" alt="Boeing B738" width="256" />
