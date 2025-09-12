#include "pap3_demo.h"

#include "pap3_device.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <XPLMProcessing.h>

namespace pap3::device {

    // Map one instance per X-Plane flight loop refcon
    static float PAP3Demo_FlightLoop(float inElapsedSinceLastCall, float, int, void *refcon) {
        auto *self = static_cast<PAP3Demo *>(refcon);
        if (!self) {
            return 0.0f;
        }
        return self->FlightLoopThunk(inElapsedSinceLastCall, 0.0f, 0, refcon);
    }

    PAP3Demo::PAP3Demo(PAP3Device *dev) :
        _dev(dev), _rng(std::random_device{}()) {
    }

    PAP3Demo::~PAP3Demo() {
        stop();
    }

    void PAP3Demo::start(const std::vector<std::uint8_t> &ledIds, float lcdFps, float ledFps, float hbFps) {
        if (_running || !_dev) {
            return;
        }
        _ledIds = ledIds;
        if (_ledIds.size() < 2) {
            // Provide a fallback simple range if nothing was passed.
            _ledIds.clear();
            for (uint8_t i = 0; i < 16; ++i) {
                _ledIds.push_back(i);
            }
        }

        _dtLcd = (lcdFps > 0.f) ? 1.0 / lcdFps : 1.0 / 6.0;
        _dtLed = (ledFps > 0.f) ? 1.0 / ledFps : 1.0 / 5.0;
        _dtHb = (hbFps > 0.f) ? 1.0 / hbFps : 1.0 / 1.2;

        _accumLcd = _accumLed = _accumHb = 0.0;
        _col = 0;
        _phase = 0.0;

        // Init LCD (best effort)
        _dev->lcdInit();

        // Register flight loop
        XPLMRegisterFlightLoopCallback(PAP3Demo_FlightLoop, -1.0f, this);
        _running = true;
    }

    void PAP3Demo::stop() {
        if (!_running) {
            return;
        }

        // Turn off any LEDs we may have left ON (best effort)
        for (auto id : _ledIds) {
            _dev->setLed(id, false);
        }

        XPLMUnregisterFlightLoopCallback(PAP3Demo_FlightLoop, this);
        _running = false;
    }

    float PAP3Demo::FlightLoopThunk(float inElapsedSinceLastCall, float, int, void *refcon) {
        auto *self = static_cast<PAP3Demo *>(refcon);
        if (!self || !self->_running) {
            return 0.0f;
        }
        return self->updateTick(inElapsedSinceLastCall);
    }

    float PAP3Demo::updateTick(float elapsedSinceLastCall) {
        _accumLcd += elapsedSinceLastCall;
        _accumLed += elapsedSinceLastCall;
        _accumHb += elapsedSinceLastCall;

        if (_accumLcd >= _dtLcd) {
            _accumLcd -= _dtLcd;
            lcdStepMarquee();
        }
        if (_accumLed >= _dtLed) {
            _accumLed -= _dtLed;
            ledStepChase();
        }
        if (_accumHb >= _dtHb) {
            _accumHb -= _dtHb;
            heartbeatStep();
        }

        // Run again next frame
        return -1.0f;
    }

    // ------------------------- LCD marquee -------------------------

    /*
     * Digit layout (12 slots) across MCP 7-seg, based on groups:
     *  - SPD uses G1 masks: 0x08 (kilo), 0x04 (hund), 0x02 (tens), 0x01 (units)
     *  - Captain CRS uses G1 upper: 0x80, 0x40, 0x20
     *  - HDG uses G2: 0x40, 0x20, 0x10
     *  - FO CRS uses G4: 0x40, 0x20, 0x10
     *
     * This gives 4 + 3 + 3 + 3 = 13 slots; we’ll use 12 contiguous ones for a clean marquee.
     * Adjust to your actual panel if needed.
     */
    static const PAP3Demo::DigitSlot kSlots[12] = {
        {PAP3Demo::SegGroupId::G1, 0x08}, // SPD kilo
        {PAP3Demo::SegGroupId::G1, 0x04}, // SPD hund
        {PAP3Demo::SegGroupId::G1, 0x02}, // SPD tens
        {PAP3Demo::SegGroupId::G1, 0x01}, // SPD units
        {PAP3Demo::SegGroupId::G1, 0x80}, // CRS C hund
        {PAP3Demo::SegGroupId::G1, 0x40}, // CRS C tens
        {PAP3Demo::SegGroupId::G1, 0x20}, // CRS C units
        {PAP3Demo::SegGroupId::G2, 0x40}, // HDG hund
        {PAP3Demo::SegGroupId::G2, 0x20}, // HDG tens
        {PAP3Demo::SegGroupId::G2, 0x10}, // HDG units
        {PAP3Demo::SegGroupId::G4, 0x40}, // CRS FO hund
        {PAP3Demo::SegGroupId::G4, 0x20}, // CRS FO tens
        // (CRS FO units skipped to keep 12 slots)
    };

    void PAP3Demo::lcdStepMarquee() {
        std::vector<std::uint8_t> p = buildLcdPayloadForWindow(_col);

        // Correct LCD transaction:
        _dev->lcdSendPayload(p);
        _dev->lcdSendEmpty();
        _dev->lcdSendEmpty();
        _dev->lcdCommit();

        ++_col;
        const int span = static_cast<int>(_text.size()) + _lineLen;
        if (_col >= span) {
            _col = 0;
        }
    }

    std::vector<std::uint8_t> PAP3Demo::buildLcdPayloadForWindow(int startCol) const {
        // 32-byte payload for absolute 0x19..0x38 inclusive
        std::vector<std::uint8_t> p(32, 0x00);

        // Segment absolute offsets per the reverse engineering used in the project:
        struct SegGroup {
                uint8_t g, f, e, d, c, b, a;
        };

        const SegGroup G1{0x1D, 0x21, 0x25, 0x29, 0x2D, 0x31, 0x35};
        const SegGroup G2{0x1E, 0x22, 0x26, 0x2A, 0x2E, 0x32, 0x36};
        const SegGroup G3{0x1F, 0x23, 0x27, 0x2B, 0x2F, 0x33, 0x37};
        const SegGroup G4{0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38};

        auto segsOf = [&](SegGroupId id) -> const SegGroup & {
            switch (id) {
                case SegGroupId::G1:
                    return G1;
                case SegGroupId::G2:
                    return G2;
                case SegGroupId::G3:
                    return G3;
                default:
                    return G4;
            }
        };

        // Window the text across 12 slots
        const int span = static_cast<int>(_text.size()) + _lineLen;
        for (int i = 0; i < _lineLen; ++i) {
            const int col = startCol + i - _lineLen; // negative means pre-roll
            char ch = ' ';
            if (col >= 0 && col < static_cast<int>(_text.size())) {
                ch = _text[col];
            }

            const uint8_t pat = segMaskForChar(ch);
            const DigitSlot &slot = kSlots[i];
            const SegGroup &grp = segsOf(slot.group);
            // Pattern bits: a..g map to grp.a..grp.g
            auto orAbs = [&](uint8_t absOffset, uint8_t bit) {
                orSeg(p, absOffset, bit);
            };
            if (pat & 0b00000010) {
                orAbs(grp.a, slot.mask); // A
            }
            if (pat & 0b00000100) {
                orAbs(grp.b, slot.mask); // B
            }
            if (pat & 0b00001000) {
                orAbs(grp.c, slot.mask); // C
            }
            if (pat & 0b00010000) {
                orAbs(grp.d, slot.mask); // D
            }
            if (pat & 0b00100000) {
                orAbs(grp.e, slot.mask); // E
            }
            if (pat & 0b01000000) {
                orAbs(grp.f, slot.mask); // F
            }
            if (pat & 0b10000000) {
                orAbs(grp.g, slot.mask); // G
            }
        }

        return p;
    }

    uint8_t PAP3Demo::segMaskForChar(char c) {
        // 7-seg patterns (bit layout: a=0b00000010 … g=0b10000000)
        switch (c) {
            case 'H':
            case 'h':
                return 0b11001110; // H ~ b c e f g
            case 'E':
            case 'e':
                return 0b11110010; // E ~ a d e f g
            case 'L':
            case 'l':
                return 0b00111000; // L ~ d e f
            case 'O':
            case 'o':
                return 0b01111110; // O ~ a b c d e f (g off)
            case ' ':
                return 0b00000000;
            default:
                return 0b00000000;
        }
    }

    void PAP3Demo::orSeg(std::vector<std::uint8_t> &p, uint8_t absOffset, uint8_t bit) const {
        const int idx = int(absOffset) - 0x19;
        if (idx >= 0 && idx < (int) p.size()) {
            p[idx] |= bit;
        }
    }

    // ------------------------- LED chase -------------------------

    void PAP3Demo::ledStepChase() {
        if (_ledIds.size() < 2) {
            return;
        }

        // Remember previous pair to avoid strobing all LEDs every tick
        static std::size_t prevI = _ledIds.size(); // invalid sentinel
        static std::size_t prevJ = _ledIds.size();

        // Turn off only the previous pair (best effort)
        if (prevI < _ledIds.size()) {
            _dev->setLed(_ledIds[prevI], false);
        }
        if (prevJ < _ledIds.size()) {
            _dev->setLed(_ledIds[prevJ], false);
        }

        // Move forward with light jitter to make it feel alive
        std::uniform_int_distribution<int> jitter(-1, +1);
        _ledIndex = (_ledIndex + 1) % _ledIds.size();
        std::size_t i = _ledIndex;
        std::size_t j = (_ledIndex + 1 + _ledIds.size() + jitter(_rng)) % _ledIds.size();

        _dev->setLed(_ledIds[i], true);
        _dev->setLed(_ledIds[j], true);

        prevI = i;
        prevJ = j;
    }

    // ------------------------- Heartbeat -------------------------

    void PAP3Demo::heartbeatStep() {
        // Smooth sine-based heartbeat around ~0.6–1.0 Hz (set by _dtHb above)
        const double freq = 0.6; // Hz
        _phase += _dtHb * freq;
        if (_phase >= 1.0) {
            _phase -= 1.0;
        }

        const double s = 0.5 * (1.0 + std::sin(2.0 * M_PI * _phase)); // 0..1
        const int v = int(std::round(_hbMin + s * (_hbMax - _hbMin)));
        const uint8_t val = static_cast<uint8_t>(std::clamp(v, 0, 255));

        // 0=backlight, 1=LCD; do not change LED channel 2
        _dev->setDimming(0, val);
        _dev->setDimming(1, val);
    }

}