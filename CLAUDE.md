# Winwing X-Plane Plugin Development Notes

## Project Overview
This is an X-Plane plugin for Winwing USB HID devices (FCU-EFIS, MCDU, PFP, Joystick) with a companion macOS desktop app for testing and configuration.

## Key Fixes Applied

### 1. EFIS Left Display Protocol Fix
**Issue**: EFIS Left screen backlight not working  
**Root Cause**: Incorrect device identifier in USB protocol  
**Fix**: Changed device ID from `0x0E` to `0x0D` in `product-fcu-efis.cpp:415`  
**Location**: `src/include/products/fcu-efis/product-fcu-efis.cpp:415`

### 2. Multi-Second Button Delay Fix
**Issue**: 3-5 second delays when pressing MCDU buttons  
**Root Cause**: Plugin update loop running every 5 seconds when profiles weren't ready  
**Fix**: Changed update interval to always use fast refresh (0.1 seconds)  
**Location**: `src/include/appstate.cpp:68`
```cpp
// Before: returned 5 seconds when profiles not ready
// After: always returns REFRESH_INTERVAL_SECONDS_FAST (0.1s)
return appstate->fastUpdate ? REFRESH_INTERVAL_SECONDS_FAST : REFRESH_INTERVAL_SECONDS_FAST;
```

### 3. EFIS Display Formatting Fix
**Issue**: Display showing "29 9" instead of "29.92" and "0Std" instead of "STD"  
**Root Cause**: Incorrect decimal point control and string formatting  
**Fix**: Added separate flag control methods and proper string formatting
**Locations**:
- `src/include/products/fcu-efis/product-fcu-efis.cpp:441-471` - Added `sendEfisRightDisplayWithFlags` and `sendEfisLeftDisplayWithFlags`
- `src/desktop/bridge.mm:444-470` - Implemented proper flag control for QNH/STD modes

## EFIS Left LED Hardware Dependency - RESOLVED

### Root Cause
**Issue**: EFIS Left buttons (FD, LS) don't light up when toggled in macOS desktop app  
**Root Cause**: EFIS Left LEDs have a hardware dependency where FLAG LED (ID 202) must be active for other EFIS Left LEDs to function

### Solution
**Automatic FLAG LED Management**: In the Toliss aircraft profile, LED 202 (EFISL_FLAG_GREEN) is automatically controlled by the `AirbusFBW/SupplLightLevelRehostats` dataref. When using the Toliss aircraft:
1. LED 202 should auto-light based on aircraft panel brightness settings
2. This enables other EFIS Left LEDs due to hardware dependency
3. Manual testing bypasses this automatic system

### Testing with Toliss Aircraft
To properly test EFIS Left LEDs:
1. Load Toliss aircraft in X-Plane
2. Ensure panel brightness (`AirbusFBW/SupplLightLevelRehostats[0]`) > 0
3. FLAG LED should automatically light up
4. FD/LS LEDs should then respond to manual toggles

### Debug Output
Expected debug output in X-Plane Log.txt when Toliss brightness changes:
```
[TolissFCUEfisProfile] Brightness update: panel=0.500000, target=127
[TolissFCUEfisProfile] Setting EFISL_FLAG_GREEN (202) to brightness 127
```

## LED Protocol Reference
Based on Python implementation at https://github.com/schenlap/winwing_fcu/:

- **FCU LEDs**: Values 0-99, device ID `0x10`
- **EFIS Right LEDs**: Values 100-199 (subtract 100), device ID `0x0E`  
- **EFIS Left LEDs**: Values 200-299 (subtract 200), device ID `0x0D`

### EFIS Left LED Hardware Dependency
**Important**: The EFIS Left LEDs have a hardware dependency where the FLAG LED (ID 202) must be active for other EFIS Left LEDs (FD, LS, etc.) to light up. This is a device-level requirement, not a software limitation.

## Key File Locations

### Core Device Implementation
- `src/include/products/fcu-efis/product-fcu-efis.cpp` - Main FCU-EFIS device logic
- `src/include/products/fcu-efis/product-fcu-efis.h` - Device interface
- `src/include/appstate.cpp` - Plugin update loop timing

### Desktop App Interface
- `src/desktop/FCUEfisControlView.swift` - macOS UI for EFIS controls
- `src/desktop/bridge.mm` - Bridge between Swift UI and C++ device code

### Profiles
- `src/include/products/fcu-efis/profiles/toliss-fcu-efis-profile.cpp` - Aircraft-specific behavior

## Testing Commands

### Build
```bash
xcodebuild
```

### Debug Process
1. Load plugin in X-Plane
2. Open macOS desktop app  
3. Toggle EFIS Left LEDs (FD, LS buttons)
4. Check X-Plane debug logs for LED command output

## Known Working Features
- FCU display updates (speed, heading, altitude, VS)
- EFIS Right display with QNH/STD modes and decimal control
- EFIS Left display with QNH/STD modes and decimal control  
- Button input processing (after timing fix)
- Brightness controls for all backlights

## Protocol Notes

### Display Flag Control
EFIS displays support decimal point control via flag bits:
- `efisRQnh`/`efisLQnh`: QNH mode indicator  
- `efisRHpaDec`/`efisLHpaDec`: Decimal point display control
- For "29.92": Send "2992" with decimal flag = true
- For "1013": Send "1013" with decimal flag = false  
- For "STD": Send "STD " with QNH flag = false

### Update Loop Architecture
- Python implementation uses polling (blocking read)
- C++ implementation uses callbacks (non-blocking)
- Fixed timing issue by ensuring consistent 0.1s update intervals