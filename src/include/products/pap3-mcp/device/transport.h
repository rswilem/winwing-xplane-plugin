// src/include/products/pap3-mcp/device/transport.h
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

/*
    Transport layer for PAP3 MCP.

    Scope:
      - Builds HID frames for PAP3 (LCD, LEDs, dimming) and delegates the actual write to a pluggable writer.
      - Keeps the USB/HID details out of the public header (no heavy includes), enabling unit testing.

    Design:
      - A low-level writer is installed once (see setWriter). All send* helpers go through that indirection.
      - The writer receives the raw payload (e.g., 64 bytes for LCD) and is responsible for platform quirks,
        such as prefixing a HID Report ID on macOS before calling into the real device primitive.

    Notes:
      - LCD frames are 64 bytes long (payload built here). The writer may add 1 byte for Report ID on macOS.
      - Sequence byte is maintained non-zero (1..255). The helpers will bump it after a successful send.
      - No DataRefs here; avionics profiles own sim-side logic. This layer only formats bytes for the device.
*/

class USBDevice;

namespace pap3::device::transport {

using DevicePtr = ::USBDevice*;

// Low-level writer signature: writes exactly 'len' bytes from 'data' to 'dev'.
// Returns true on success. The writer may adapt the buffer (e.g., add Report ID) before calling HID APIs.
using WriterFn = bool(*)(DevicePtr, const uint8_t*, std::size_t);

// Install the concrete writer (call once after the HID device is opened).
void setWriter(WriterFn fn);

// Default writer for macOS HID: prefixes Report ID (0x00 by default) and calls USBDevice::writeData(std::vector<uint8_t>).
bool writerUsbWriteData(DevicePtr dev, const uint8_t* data, std::size_t len);

// -----------------------------------------------------------------------------
// Public transport API
// -----------------------------------------------------------------------------

// Brightness (backlight/LCD/LEDs). Common format:
//   02 0F BF 00 00 03 49 CC VV 00 00 00 00 00
// CC = channel (0/1/2), VV = 0..255
bool sendDimming(DevicePtr dev, uint8_t channel, uint8_t value);

// LED on/off. Same base format; CC = hardware LED id, VV = 0x01/0x00.
bool sendLed(DevicePtr dev, uint8_t ledId, bool on);

// LCD payload frame (opcode 0x38):
// - Common header at [0x00..0x03]
// - Fixed preamble at [0x04..]
// - 32-byte user payload copied to [0x19..0x38] inclusive
// On success, increments 'seq' (kept non-zero).
bool sendLcdPayload(DevicePtr dev, uint8_t& seq, const std::vector<uint8_t>& payloadAfter0x19);

// LCD "empty" frame (opcode 0x38) with zeros after common header.
// On success, increments 'seq'.
bool sendLcdEmptyFrame(DevicePtr dev, uint8_t& seq);

// LCD commit frame (opcode 0x2A) with fixed constants at specific offsets.
// On success, increments 'seq'.
bool sendLcdCommit(DevicePtr dev, uint8_t& seq);

// LCD init frame (opcode 0x12) with a fixed tail at [0x04..].
// On success, increments 'seq'.
bool sendLcdInit(DevicePtr dev, uint8_t& seq);

// AT solenoid latch. Exact 14-byte frame:
// 02 0F BF 00 00 03 49 1E VV 00 00 00 00 00
// VV = 0x01 (ON) or 0x00 (OFF)
bool sendATSolenoid(DevicePtr dev, bool on);

} // namespace pap3::device::transport