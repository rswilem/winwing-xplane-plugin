Font Management System for WINCTRL FMC (MCDU or PFP 3N / PFP 4 / PFP 7)

The plugin is able to read out this folder and load custom fonts
into the FMC in X-Plane.
Please also see https://rswilem.github.io/winctrl-font-editor/ for creating custom fonts.

## Installation

### Folder Location
Custom fonts must be placed in the following directory:
`X-Plane root directory/Resources/plugins/winctrl/fonts/`
You have found this README.txt file there.

### Creating Custom Fonts
1. Visit https://rswilem.github.io/winctrl-font-editor/
2. Design or upload your custom font
3. Download the generated .xpwwf file
4. Place the .xpwwf file in the fonts folder specified above

### Selecting Fonts in X-Plane
1. Open X-Plane
2. Navigate to: Plugins > WINCTRL > FMC > Display font
3. Choose from available custom fonts or select "Managed by plugin" or "No custom font"
4. The selection will automatically persist across all aircraft

### Font Options
- **Managed by plugin (default)**: Uses the plugin's built-in fonts, automatically adjusted
  to be realistic for the currently selected aircraft
- **No custom font**: Reverts to the default font included with the WINCTRL FMC hardware.
  Please note, this is only on start-up. You will need to disconnect and reconnect the FMC
  to see changes if switching from a custom font.
- **Custom Fonts**: Any .xpwwf file placed in the fonts directory

## Behavior
- Custom font preferences are saved globally and apply to all aircraft
- Font changes take effect immediately, except when switching to "No custom font"
  which requires a device reconnect
