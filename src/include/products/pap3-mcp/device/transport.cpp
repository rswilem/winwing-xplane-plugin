// src/include/products/pap3-mcp/device/transport.cpp
//
// PAP3 MCP - USB/HID transport layer
// -----------------------------------
// This module centralizes every HID frame needed to control the PAP3 MCP device:
//  • Dimming channels
//  • LEDs
//  • A/T Solenoid
//  • LCD payloads, commits and init frames
//
// WARNING: These packet formats are reverse-engineered. Do NOT change bytes,
//          offsets, or sequence logic unless you know the protocol details.
//

#include "transport.h"    // public API + forward-declared USBDevice
#include "usbdevice.h"    // full type needed only here

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

#include <XPLMUtilities.h>

namespace pap3::device::transport {

// -----------------------------------------------------------------------------
// Writer management
// -----------------------------------------------------------------------------

static WriterFn s_writer = nullptr; // installed writer

/// Checks if a writer function has been registered.
static inline bool haveWriter() noexcept { return s_writer != nullptr; }

/// Calls the installed writer. Returns false if none is installed.
static inline bool write(DevicePtr dev, const uint8_t* data, std::size_t len) noexcept
{
    return (s_writer) ? s_writer(dev, data, len) : false;
}

/// Ensures the sequence byte never wraps to zero (protocol requirement).
static inline void bumpSeq(uint8_t& seq) noexcept
{
    if (seq == 0) seq = 1;
    else if (++seq == 0) seq = 1;
}

bool writerUsbWriteData(DevicePtr dev, const uint8_t* data, std::size_t len)
{
    if (!dev || !data || len == 0) return false;
    std::vector<uint8_t> buffer(data, data + len);
    return dev->writeData(std::move(buffer));
}

void setWriter(WriterFn fn) { s_writer = fn; }

// -----------------------------------------------------------------------------
// 14-BYTE SIMPLE COMMANDS
// Used by DIMMING, LED and A/T SOLENOID control
// -----------------------------------------------------------------------------
namespace {

constexpr std::size_t kShortLen = 14;

/// Offsets in the 14-byte packet
enum ShortOffsets : std::size_t {
    IDX_REPORT_ID = 0x00,
    IDX_0F        = 0x01,
    IDX_BF        = 0x02,
    IDX_ZERO0     = 0x03,
    IDX_ZERO1     = 0x04,
    IDX_CONST_03  = 0x05,
    IDX_CONST_49  = 0x06,
    IDX_SELECTOR  = 0x07, // Feature selector (channel, LED ID, solenoid)
    IDX_VALUE     = 0x08  // Value for the feature
    // Remaining bytes 0x09..0x0D are always 0x00
};

/// Constant values for the common frame
constexpr uint8_t REPORT_ID  = 0x02;
constexpr uint8_t HEADER_0F  = 0x0F;
constexpr uint8_t HEADER_BF  = 0xBF;
constexpr uint8_t CONST_03   = 0x03;
constexpr uint8_t CONST_49   = 0x49;

/// Builds the base 14-byte packet.
/// @param selector  Byte at offset 0x07: selects the hardware feature
/// @param value     Byte at offset 0x08: command value for the feature
inline std::array<uint8_t, kShortLen> makeShortCommand(uint8_t selector, uint8_t value)
{
    std::array<uint8_t, kShortLen> buf{};
    buf[IDX_REPORT_ID] = REPORT_ID;
    buf[IDX_0F]        = HEADER_0F;
    buf[IDX_BF]        = HEADER_BF;
    buf[IDX_ZERO0]     = 0x00;
    buf[IDX_ZERO1]     = 0x00;
    buf[IDX_CONST_03]  = CONST_03;
    buf[IDX_CONST_49]  = CONST_49;
    buf[IDX_SELECTOR]  = selector;
    buf[IDX_VALUE]     = value;
    // remaining bytes are 0x00
    return buf;
}

} // namespace

// -----------------------------------------------------------------------------
// Public API - Short commands
// -----------------------------------------------------------------------------

/// DIMMING control
/// Builds and sends a 14-byte frame:
///   02 0F BF 00 00 03 49 [channel] [value] 00 00 00 00 00
/// channel: 0..2 (dimming channels)
/// value:   0..255 (brightness)
bool sendDimming(DevicePtr dev, uint8_t channel, uint8_t value)
{
    if (!haveWriter()) return false;
    auto buf = makeShortCommand(channel, value);
    debug_force("[PAP3] sendDimming(ch=%u, val=%u) sent\n", static_cast<unsigned>(channel), static_cast<unsigned>(value));
    return write(dev, buf.data(), buf.size());
    
}

/// LED control
/// Builds and sends a 14-byte frame:
///   02 0F BF 00 00 03 49 [ledId] [0x01=ON | 0x00=OFF] 00 00 00 00 00
bool sendLed(DevicePtr dev, uint8_t ledId, bool on)
{
    if (!haveWriter()) return false;
    auto buf = makeShortCommand(ledId, static_cast<uint8_t>(on ? 0x01 : 0x00));
    debug_force("[PAP3] sendLed(id=0x%02X, %s) sent\n", static_cast<unsigned>(ledId), on ? "ON" : "OFF");
    return write(dev, buf.data(), buf.size());
    
}

/// A/T Solenoid control
/// Builds and sends a 14-byte frame:
///   02 0F BF 00 00 03 49 1E [0x01=ON | 0x00=OFF] 00 00 00 00 00
bool sendATSolenoid(DevicePtr dev, bool on)
{
    if (!haveWriter()) return false;
    constexpr uint8_t selectorAT = 0x1E;
    auto buf = makeShortCommand(selectorAT, static_cast<uint8_t>(on ? 0x01 : 0x00));
    debug_force("[PAP3] sendATSolenoid(%s) sent\n", on ? "ON" : "OFF");
    return write(dev, buf.data(), buf.size());
    
}

// -----------------------------------------------------------------------------
// 64-BYTE LCD COMMANDS
// -----------------------------------------------------------------------------
namespace {

constexpr std::size_t kReportLen = 64;

/// Header offsets for every LCD frame
enum HeaderOffsets : std::size_t {
    HDR_F0   = 0x00,
    HDR_00   = 0x01,
    HDR_SEQ  = 0x02,
    HDR_OP   = 0x03,
    HDR_NEXT = 0x04 // first byte after header
};

/// Opcodes for LCD operations
enum LcdOpcodes : uint8_t {
    OP_LCD_PAYLOAD = 0x38,
    OP_LCD_COMMIT  = 0x2A,
    OP_LCD_INIT    = 0x12
};

// LCD PAYLOAD preamble (starts at offset 0x04)
static constexpr uint8_t kPayloadPreamble[] = {
    0x0F, 0xBF, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00,
    0xDF, 0xA2, 0x50, 0x00, 0x00, 0xB0
};

// User payload range for LCD frames (0x19..0x38 inclusive)
constexpr std::size_t kPayloadStart = 0x19;
constexpr std::size_t kPayloadEnd   = 0x38;
constexpr std::size_t kPayloadMax   = (kPayloadEnd - kPayloadStart + 1);

// LCD COMMIT constants (scattered offsets)
inline void writeCommitConstants(uint8_t* b)
{
    b[0x1D] = 0x0F;
    b[0x1E] = 0xBF;
    b[0x21] = 0x03;
    b[0x22] = 0x01;
    b[0x25] = 0xDF;
    b[0x26] = 0xA2;
    b[0x27] = 0x50;
}

// LCD INIT tail (starts at offset 0x04)
static constexpr uint8_t kInitTail[] = {
    0x0F, 0xBF, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00,
    0x26, 0xCC, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00
};

// Helpers
inline void zero64(uint8_t* b) { std::memset(b, 0, kReportLen); }

inline void initCommon(uint8_t* b, uint8_t seq, uint8_t opcode)
{
    zero64(b);
    b[HDR_F0]  = 0xF0;
    b[HDR_00]  = 0x00;
    b[HDR_SEQ] = (seq == 0) ? 0x01 : seq;
    b[HDR_OP]  = opcode;
}

inline void writePayloadPreamble(uint8_t* b)
{
    std::memcpy(b + HDR_NEXT, kPayloadPreamble, sizeof(kPayloadPreamble));
}

inline void writePayloadBytes(uint8_t* b, const std::vector<uint8_t>& payload)
{
    const auto n = std::min(payload.size(), kPayloadMax);
    if (n > 0) {
        std::memcpy(b + kPayloadStart, payload.data(), n);
    }
}

inline void writeInitTail(uint8_t* b)
{
    std::memcpy(b + HDR_NEXT, kInitTail, sizeof(kInitTail));
}

} // namespace

/// Sends an LCD payload frame (opcode 0x38)
/// Frame layout:
///   Common header
///   Payload preamble at 0x04
///   User data bytes at 0x19..0x38
bool sendLcdPayload(DevicePtr dev, uint8_t& seq, const std::vector<uint8_t>& payload)
{
    if (!haveWriter()) return false;

    uint8_t buf[kReportLen];
    initCommon(buf, seq, OP_LCD_PAYLOAD);
    writePayloadPreamble(buf);
    writePayloadBytes(buf, payload);

    const bool ok = write(dev, buf, sizeof(buf));
    if (ok) bumpSeq(seq);
    debug_force("[PAP3][LCD] sendLcdPayload(seq=%u) sent\n", static_cast<unsigned>(seq));
    return ok;
}

/// Sends an empty LCD frame (opcode 0x38) with no user payload
bool sendLcdEmptyFrame(DevicePtr dev, uint8_t& seq)
{
    if (!haveWriter()) return false;

    uint8_t buf[kReportLen];
    initCommon(buf, seq, OP_LCD_PAYLOAD);

    const bool ok = write(dev, buf, sizeof(buf));
    if (ok) bumpSeq(seq);
    debug_force("[PAP3][LCD] sendLcdEmptyFrame(seq=%u) sent\n", static_cast<unsigned>(seq));
    return ok;
}

/// Sends an LCD COMMIT frame (opcode 0x2A)
/// This signals the device to apply the buffered frames
bool sendLcdCommit(DevicePtr dev, uint8_t& seq)
{
    if (!haveWriter()) return false;

    uint8_t buf[kReportLen];
    initCommon(buf, seq, OP_LCD_COMMIT);
    writeCommitConstants(buf);

    const bool ok = write(dev, buf, sizeof(buf));
    debug_force("[PAP3][LCD] sendLcdCommit(seq=%u) sent\n", static_cast<unsigned>(seq));
    if (ok) bumpSeq(seq);
    return ok;
}

/// Sends an LCD INIT frame (opcode 0x12)
/// Initializes the LCD before sending payloads
bool sendLcdInit(DevicePtr dev, uint8_t& seq)
{
    if (!haveWriter()) return false;

    uint8_t buf[kReportLen];
    initCommon(buf, seq, OP_LCD_INIT);
    writeInitTail(buf);

    const bool ok = write(dev, buf, sizeof(buf));
    debug_force("[PAP3][LCD] sendLcdInit(seq=%u) sent\n", static_cast<unsigned>(seq));
    if (ok) bumpSeq(seq);
    return ok;
}

} // namespace pap3::device::transport