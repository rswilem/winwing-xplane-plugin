# FCU-EFIS Display Protocol Documentation

This document outlines how to write to the Winwing FCU-EFIS hardware displays based on our implementation experience.

## Hardware Overview

The FCU-EFIS unit contains multiple 7-segment displays:
- **Speed/Mach Display**: 3 digits with decimal point support
- **Heading Display**: 3 digits 
- **Altitude Display**: 5 digits
- **Vertical Speed Display**: 4 digits with sign and decimal point support
- **EFIS Left Display**: QNH/STD with decimal point
- **EFIS Right Display**: QNH/STD with decimal point

## Protocol Basics

### USB Device IDs
- **FCU LEDs**: Device ID `0x10`, LED values 0-99
- **EFIS Right LEDs**: Device ID `0x0E`, LED values 100-199 (subtract 100)  
- **EFIS Left LEDs**: Device ID `0x0D`, LED values 200-299 (subtract 200)

### Display Data Format
Displays expect 4-byte sequences where each byte contains:
- **Character data**: 7-segment encoded value
- **Flag bits**: Control decimal points, signs, and indicators

## Display Formatting Rules

### Speed/Mach Display
```cpp
// Speed mode: Send as 3-digit integer
"250" -> displays "250"

// Mach mode: Send hundredths without decimal
float mach = 0.82f;
int machValue = (int)round(mach * 100); // 82
string display = "082"; // displays as "0.82" with decimal flag
```

**Gotchas:**
- Must detect Mach mode using `sim/cockpit/autopilot/airspeed_is_mach`
- Set `machComma` flag for decimal point in Mach mode
- Never include sign in string - always use absolute values

### Heading Display
```cpp
// Convert 360 to 0 for display (aviation standard)
int heading = 360;
if (heading == 360) heading = 0;
string display = "000"; // displays "000"
```

**Gotchas:**
- Heading 360 should display as 0
- Always format as 3 digits with leading zeros

### Altitude Display
```cpp
// Always send 5-digit value with leading zeros
int altitude = 10000;
string display = "10000"; // displays "10000"
```

### Vertical Speed Display

#### VS Mode (HDG/VS)
```cpp
// Send 4 digits without sign in string
int vs = -1500;
string display = "1500"; // absolute value
bool vsSign = (vs >= 0); // sign controlled by flag

// Special case: multiples of 100 show "##"
int vs = 1000;
string display = "10##"; // displays "10oo" (## appears as oo on hardware)
```

#### FPA Mode (TRK/FPA)
```cpp
// Dataref value represents feet, divide by 1000 for display
float datarefValue = 600; // from sim
float fpaValue = datarefValue / 1000.0f; // 0.6
int fpaTenths = (int)round(abs(fpaValue) * 10); // 6
string display = "06  "; // displays as "0.6" with decimal

// Examples:
// 0.0 -> "00  " (displays "0.0")
// 0.1 -> "01  " (displays "0.1")
// 0.6 -> "06  " (displays "0.6")
// 1.2 -> "12  " (displays "1.2")
// 2.5 -> "25  " (displays "2.5")
```

#### VS Dashes
```cpp
// When VS is dashed, show with minus sign
string display = "-----";
bool vsSign = false; // forces minus sign display
```

**Major Gotchas:**
- **Never include sign characters in display strings** - use `vsSign` flag
- FPA values: multiply by 10 and format as 2 digits + spaces (e.g., "06  " for 0.6)
- VS sign controlled by V2 bit 0x10 (matches Python implementation)
- vsVerticalLine (V2 bit 0x20) must be disabled when VS shows dashes to avoid comma
- Hardware decimal point is controlled by `fpaComma` flag, not string format

### EFIS Displays (QNH/STD)

#### QNH Mode
```cpp
// For pressure values like 29.92 or 1013
float qnh = 29.92f;
if (inHg) {
    // Send as "2992" with decimal flag
    int qnhValue = (int)round(qnh * 100);
    string display = "2992"; // displays "29.92"
    bool decimal = true;
} else {
    // Send as "1013" without decimal
    string display = "1013"; // displays "1013"  
    bool decimal = false;
}
```

#### STD Mode
```cpp
string display = "STD "; // pad to 4 chars, displays "STD"
bool qnhFlag = false; // disables QNH indicator
```

**Gotchas:**
- Always pad STD to 4 characters with trailing space
- QNH decimal position depends on units (inHg vs hPa)
- Must control both decimal flag AND QNH mode flag

## Flag Bit Mapping

### Speed Display Flags
- `machComma`: Decimal point for Mach display (S1 byte, bit 0x01)

### Vertical Speed Flags  
- `vsSign`: Sign display (V2 byte, bit 0x10) - true = positive, false = negative
- `vsVerticalLine`: Vertical line/comma (V2 byte, bit 0x20) - disable for dashed VS
- `fpaComma`: Decimal point for FPA (V3 byte, bit 0x10)
- `vsIndication`: VS mode indicator (V0 byte, bit 0x40)
- `fpaIndication`: FPA mode indicator (V0 byte, bit 0x80)

### EFIS Display Flags
- `efisRQnh`/`efisLQnh`: QNH mode indicators
- `efisRHpaDec`/`efisLHpaDec`: Decimal point control

## Hardware Dependencies

### EFIS Left LED Dependency
The EFIS Left LEDs have a hardware dependency where **LED 202 (FLAG_GREEN) must be active** for other EFIS Left LEDs (FD, LS) to function. This is controlled automatically by aircraft brightness settings in Toliss profiles.

## Common Mistakes

1. **Including signs in display strings** - Always use absolute values and control signs with flags
2. **Wrong FPA formatting** - Use tenths (multiply by 10) and format as 2 digits + spaces
3. **Wrong VS sign bit** - Use V2 0x10, not V3 0x20 (check Python implementation)
4. **vsVerticalLine in dashed VS** - Must disable to avoid unwanted comma
5. **Heading 360** - Convert to 0 for display (aviation standard)
6. **Baro adjustment in hPa** - Need custom handling for 1 hPa increments

## Custom Baro Adjustment

The Toliss profile includes custom handling for barometric pressure adjustment:
- In **hPa mode**: Each knob click adjusts by 1 hPa (e.g., 1013 -> 1014)
- In **inHg mode**: Uses X-Plane's default increment (typically 0.01 inHg)
- Automatically detects the current unit mode and applies appropriate step size

## Testing Tips

1. Test both positive and negative values for signed displays
2. Verify decimal points appear in correct positions  
3. Check boundary conditions (0.0, very small/large values)
4. Test mode switches (Speed/Mach, HDG/TRK, VS/FPA)
5. Verify LED dependencies (especially EFIS Left)

## Code Locations

Key implementation files:
- **Display formatting**: `src/include/products/fcu-efis/profiles/toliss-fcu-efis-profile.cpp`
- **Hardware protocol**: `src/include/products/fcu-efis/product-fcu-efis.cpp`  
- **Flag definitions**: `src/include/products/fcu-efis/fcu-efis-aircraft-profile.h`