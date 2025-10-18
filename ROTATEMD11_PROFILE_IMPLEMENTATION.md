# Rotate MD-11 FMC Profile Implementation

## Overview
This document describes the implementation of the Rotate MD-11 FMC/CDU profile for the Winwing X-Plane plugin.

## Files Created

### 1. rotatemd11-fmc-profile.h
Location: `src/include/products/fmc/profiles/rotatemd11-fmc-profile.h`

Header file declaring the `RotateMD11FMCProfile` class that inherits from `FMCAircraftProfile`.

### 2. rotatemd11-fmc-profile.cpp
Location: `src/include/products/fmc/profiles/rotatemd11-fmc-profile.cpp`

Implementation file containing:
- Profile eligibility check (detects Rotate MD-11 aircraft)
- Display dataref mappings
- Button command mappings
- Color mapping logic
- Character mapping (special characters)
- Page update logic

## Files Modified

### 3. product-fmc.cpp
Location: `src/include/products/fmc/product-fmc.cpp`

Changes:
- Added `#include "profiles/rotatemd11-fmc-profile.h"`
- Added eligibility check and profile instantiation in `setProfileForCurrentAircraft()` method

## Implementation Details

### Display Datarefs
The profile monitors 14 lines (0-13) of CDU display data:
- Content: `Rotate/aircraft/controls/cdu_0/mcdu_line_{N}_content` (string)
- Style: `Rotate/aircraft/controls/cdu_0/mcdu_line_{N}_style` (int array)

### Button Mappings
All button commands follow the pattern: `Rotate/aircraft/controls_c/cdu_0/mcdu_key_{KEYNAME}`

Key mappings include:
- **Line Select Keys**: FL1-FL6 (left), FR1-FR6 (right)
- **Numeric Keys**: 0-9, POINT (period)
- **Alpha Keys**: A-Z
- **Function Keys**: DIR, SECFPLN, TOAPPR, REF, PERF, FPLN, INIT, PROG, MENU, FIX, NAVRAD, ENGOUT, PAGE
- **Navigation Keys**: UP, DOWN
- **Special Keys**: SPC (space), BAR (slash), CLR (clear), MINUS, PLUS

### Color Mapping
Based on style array values:
- 1: Green
- 2: Green
- 4: Amber
- 5: Green
- Also supports character-based colors: 'w'/'W' (white), 'g'/'G' (green), 'e'/'E' (amber)

### Character Mapping
Special character mappings:
- `$` → Outlined square (□)
- `*` → Degrees symbol (°)

### Font
Uses Boeing 737 font (`FontVariant::Font737`)

### Brightness Control
Uses standard X-Plane datarefs:
- `sim/cockpit/electrical/instrument_brightness` (main screen brightness)
- `sim/cockpit/electrical/avionics_on` (power state)

## Profile Detection
The profile is automatically selected when the dataref `Rotate/aircraft/controls/cdu_0/mcdu_line_0_content` exists, indicating the Rotate MD-11 aircraft is loaded.

## Build System
The CMakeLists.txt uses automatic source file discovery (`FIND_SOURCE_FILES`), so the new .cpp file is automatically included in the build without requiring manual CMakeLists.txt modifications.

## Testing Checklist
- [ ] Profile loads when Rotate MD-11 is active aircraft
- [ ] Display shows correct content from all 14 lines
- [ ] Colors render correctly (green, amber, white)
- [ ] All alphanumeric keys work
- [ ] All function keys work
- [ ] Line select keys (LSK1L-6L, LSK1R-6R) work
- [ ] Special characters display correctly
- [ ] Brightness control functions
- [ ] Profile unloads cleanly when switching aircraft

## Source References
- Python implementation: `rotate_md11/rotate_md11.py`
- Mobiflight project: `rotate_md11/Rotate_MD-11_WWPAP3_v1.0.mfproj`
- Similar profiles: zibo-fmc-profile.cpp, toliss-fmc-profile.cpp, ff777-fmc-profile.cpp

## Notes
- The profile maps some function keys to multiple hardware key identifiers (e.g., INIT mapped to both MCDU_INIT and PFP_INIT_REF) to support different hardware configurations
- The implementation follows the same patterns as other aircraft profiles in the codebase
- No linting errors were detected after implementation

