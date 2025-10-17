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

For linux, see the [Linux udev rules](#linux-udev-rules) section below to ensure proper permissions.

### Contributing

- Fork the repository.
- Download the latest X-Plane 12 SDK from https://developer.x-plane.com/sdk/plugin-sdk-downloads/.
- Unzip and copy the `SDK/` folder to the root of the repository.
- Make your changes.
- Test your changes in X-Plane 12. Datareftool plugin recommented, and make sure you uninstall FlyWithLua as it can interfere with the "Reload Plugins" functionality.
- Commit your changes and push to your fork.
- Create a pull request with a description of your changes.

### Usage

- The plugin will automatically detect your Winwing devices.
- There are no user-configurable settings at this time.

### Compatibility Matrix

The matrix below shows device and aircraft compatibility. Devices are listed vertically, aircraft horizontally.

| Device                      | Toliss A3xx | Laminar A330 | AeroGenesis A330 | Zibo 737 | IXEG 737 | FF 767/777 | SSG 747 |
| --------------------------- | ----------- | ------------ | ---------------- | -------- | -------- | ---------- | ------- |
| **URSA MINOR Joystick L+R** | 🟢          | 🟢           | 🟢               | 🟢       | 🟢       | 🟢         | 🟢      |
| **MCDU-32**                 | 🟢          | 🟢           | 🟢               | 🟢       | 🟢       | 🟢         | 🟠      |
| **PFP 3N**                  | 🟢          | 🟢           | 🟢               | 🟢       | 🟢       | 🟢         | 🟠      |
| **PFP 4**                   | 🟢          | 🟢           | 🟢               | 🟢       | 🟢       | 🟢         | 🟠      |
| **PFP 7**                   | 🟢          | 🟢           | 🟢               | 🟢       | 🟢       | 🟢         | 🟠      |
| **Panel of Autopilot 3**    | 🔴          | 🔴           | 🔴               | 🟢       | 🔴       | 🔴         | 🔴      |
| **FCU and EFIS L+R**        | 🟢          | 🟢           | 🔴               | 🔴       | 🔴       | 🔴         | 🔴      |

#### Legend

- 🟢 **Fully implemented** - All features working
- 🟠 **Partly implemented** - Some limitations or missing features
- 🔴 **Not implemented** - No support

### Known Issues

- 🟠 The SSG 747 does not expose any colour datarefs yet. Therefore, the PFP will not show the correct colours.
- 🟠 The SSG 747 has a dual FMC, but the datarefs seem to overwrite eachother.

### Credits

- [@Schenlap](https://github.com/schenlap) for the initial python implementation and providing the HID protocol documentation.
- [@zodiac1214](https://github.com/zodiac1214) for the Ursa Minor Joystick HID protocol.
- [@CyberGuerro](https://github.com/cyberguerro) for the PFP3N HID protocol and a lot of testing work.
- [@claaslange](https://github.com/claaslange) for the FCU-EFIS integration.
- [@shred86](https://github.com/shred86) for identifying the Ursa Minor Joystick R.
- [@Belnadifia](https://github.com/Belnadifia) for the complete PAP3 HID protocol and Zibo profile.
- [@ColinM9991](https://github.com/ColinM9991) for the FF777 profile.
- [@tukan68](https://github.com/tukan68) for FF767 profile.
- [@verres1](https://github.com/verres1) for enriching the FF777 profile.

### Linux udev rules

To ensure Winwing panels are accessible without root and have stable device names, create a udev rules file:

```bash
sudo nano /etc/udev/rules.d/99-winwing.rules
```

```udev
KERNEL=="hidraw*", ATTRS{idProduct}=="bc27", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-ursa-minor-l"
KERNEL=="hidraw*", ATTRS{idProduct}=="bc28", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-ursa-minor-r"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb36", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-mcdu32-cpt"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb3e", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-mcdu32-fo"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb3a", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-mcdu32-obs"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb35", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-pfp3n-cpt"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb39", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-pfp3n-fo"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb3d", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-pfp3n-obs"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb38", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-pfp4-cpt"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb40", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-pfp4-fo"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb3c", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-pfp4-obs"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb37", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-pfp7-cpt"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb3f", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-pfp7-fo"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb3b", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-pfp7-obs"
KERNEL=="hidraw*", ATTRS{idProduct}=="bb10", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-fcu"
KERNEL=="hidraw*", ATTRS{idProduct}=="bc1e", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-fcu-efis_r"
KERNEL=="hidraw*", ATTRS{idProduct}=="bc1d", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-fcu-efis_l"
KERNEL=="hidraw*", ATTRS{idProduct}=="ba01", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-fcu-efis_lr"
KERNEL=="hidraw*", ATTRS{idProduct}=="bf0f", ATTRS{idVendor}=="4098", MODE="0666", SYMLINK+="winwing-pap3"
```

### Demonstration

Also see the [X-Plane.org forum thread](https://forums.x-plane.org/files/file/95987-winwing-plugin-for-x-plane-12-mac-linux-windows/) for more information and discussion.

<img src="https://github.com/user-attachments/assets/75d4e3e0-af9e-488f-bd5e-2d834bea110d" alt="Airbus A20N" width="256" />
<img src="https://github.com/user-attachments/assets/8f5750e2-f913-479a-9f7a-6e3d6c31382d" alt="Boeing B738" width="256" />
