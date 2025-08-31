// src/include/products/pap3-mcp/device/pap3_device.h
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_map>

#include <XPLMProcessing.h> 
#include <XPLMUtilities.h>

#include "inputs.h"
#include "illumination.h"
#include "transport.h" // for transport::transport::DevicePtr


// We inherit from USBDevice so the factory in usbdevices.cpp can return this as USBDevice*.
#include "usbdevice.h"

namespace pap3 {
namespace aircraft {
    struct State;
    class PAP3AircraftProfile;
}
}

namespace pap3::device {
/**
 * PAP3Device
 *
 * Product-level facade that sends PAP3 frames via the transport module.
 * - Inherits from USBDevice so it can be returned where a USBDevice* is required.
 * - Delegates all frame formatting to transport.* helpers.
 * - Keeps sequence byte non-zero and lets transport bump it on successful sends.
 *
 * No DataRefs here. Aircraft profiles own sim-side logic and provide the values to display.
 */
class PAP3Device : public USBDevice {
public:
    // NOTE: Match this to your actual USBDevice constructor signature.
    // If USBDevice has a different constructor, adjust the base(...) call in the .cpp file.
    PAP3Device(HIDDeviceHandle hidDevice,
               uint16_t vendorId,
               uint16_t productId,
               const std::string& vendorName,
               const std::string& productName);
    ~PAP3Device() override;

    // ---------------------------
    // High-level operations
    // ---------------------------

    // LCD initialization (opcode 0x12). Returns true on success.
    bool lcdInit();

    // LCD payload (opcode 0x38): copies up to 32 bytes into absolute offsets [0x19..0x38].
    // Extra bytes in 'lcd32' are ignored; missing bytes are zero-filled by transport.
    bool lcdSendPayload(const std::vector<std::uint8_t>& lcd32);

    // LCD empty frame (opcode 0x38) with zeros after the common header.
    bool lcdSendEmpty();

    // LCD commit (opcode 0x2A). Returns true on success.
    bool lcdCommit();

    // LEDs and dimming helpers.
    // 'ledId' must be a valid PAP3 hardware LED id; 'on' toggles it.
    bool setLed(std::uint8_t ledId, bool on);

    // 'channel' is PAP3-specific (e.g., 0=backlight, 1=LCD, 2=LEDs). 'value' is 0..255.
    bool setDimming(std::uint8_t channel, std::uint8_t value);

    // AT solenoid control
    bool setATSolenoid(bool on);

    // Expose current sequence (for diagnostics/logs).
    std::uint8_t currentSeq() const noexcept { return _seq; }

    // Optional helper: direct entry-point if a lower layer already receives HID input reports.
    // Simply forwards to the internal decoder.
    void onHidInputReport(const uint8_t* report, int len);

    // USBDevice override to catch input reports if the base class surfaces them here.
    void didReceiveData(int reportId, uint8_t* report, int reportLength) override;

    // Pulse the A/T solenoid: send OFF immediately, then ON after ~50 ms.
    void pulseATSolenoid(unsigned millis = 50);

private:
    // Rolling non-zero sequence byte shared across LCD ops. Starts at 1.
    std::uint8_t _seq { 5 };

    // Ensure the transport writer is installed (idempotent).
    void ensureWriterInstalled() const;

    // Minimal startup routine called once after successful connect().
    void runStartupSequence();

    // Illumination engine and user-configured baseline values.
    pap3::device::IlluminationEngine _illum{};
    pap3::device::IlluminationConfig _illumCfg{};

    // User baseline percent for backlight when powered (0..100).
    int _userBacklightPct{100};


    void applyState(const pap3::aircraft::State& st);
    void updatePower();
    std::vector<std::uint8_t> buildLcdPayloadFrom(const pap3::aircraft::State& st) const;
    std::unique_ptr<pap3::aircraft::PAP3AircraftProfile> _profile;

    // Input decoding state
    pap3::device::Inputs _inputs;

    // FlightLoop used to re-enable solenoid after delay.
    XPLMFlightLoopID _atPulseFL{nullptr};
    bool _atPulsePending{false};
    static float ATPulseFL(float elapsed, float since, int counter, void* refcon);

    // Flight loop to poll HID at ~10 Hz (guarded by _inputFLRegistered)
    static float InputPollFL(float elapsedSinceLastCall,
                             float elapsedTimeSinceLastFlightLoop,
                             int counter,
                             void* refcon);
    bool _inputFLRegistered{false};

    // Internal: set up input callbacks and watched offsets once
    void setupInputCallbacks();

    bool _bootSwitchSyncDone{false};

    bool _initialSwitchSyncPending{true};


    struct IoCmd {
        enum Type : uint8_t {
            SetLed,
            SetDimming,
            LcdPayload,
            SetATSolenoid
        } type;
        // Petits champs utilitaires pour SetLed/SetDimming etc.
        uint8_t a = 0;   // ledId / channel
        uint8_t b = 0;   // value / on/off
        std::vector<uint8_t> payload; // LCD 32B si besoin
    };

    std::thread              _ioThread;
    std::atomic<bool>        _ioRunning{false};
    std::mutex               _ioMx;
    std::condition_variable  _ioCv;
    std::deque<IoCmd>        _ioQueue;

    // Coalescing côté worker (dernier état réellement envoyé au device)
    uint32_t                 _sentLedBitmap = 0;   // bit (ledId-0x03)
    uint8_t                  _sentDimming[3] = {255,255,255};
    bool                     _sentSolenoid = false;
    std::vector<uint8_t>     _sentLcd32;
    float                    _lastLcdPushTs = 0.f;

    // Rate-limit (LCD)
    float                    _minLcdPeriod = 1.f / 25.f; // ~25 Hz

    inline void qEnqueue(IoCmd&& c) {
        {
            std::lock_guard<std::mutex> lk(_ioMx);
            _ioQueue.emplace_back(std::move(c));
        }
        _ioCv.notify_one();
    }

    inline void qSetLed(uint8_t ledId, bool on) {
        IoCmd c; c.type = IoCmd::SetLed; c.a = ledId; c.b = on ? 1 : 0;
        qEnqueue(std::move(c));
    }
    inline void qSetDimming(uint8_t channel, uint8_t value) {
        IoCmd c; c.type = IoCmd::SetDimming; c.a = channel; c.b = value;
        qEnqueue(std::move(c));
    }
    inline void qSetSolenoid(bool on) {
        IoCmd c; c.type = IoCmd::SetATSolenoid; c.b = on ? 1 : 0;
        qEnqueue(std::move(c));
    }
    inline void qLcdPayload(const std::vector<uint8_t>& lcd32) {
        IoCmd c; c.type = IoCmd::LcdPayload; c.payload = lcd32;
        qEnqueue(std::move(c));
    }

    void ioThreadMain() {
        // Copies locales pour coalescer par "dernière intention"
        uint32_t pendLedBitmap = _sentLedBitmap;
        uint8_t  pendDim[3] = {_sentDimming[0], _sentDimming[1], _sentDimming[2]};
        bool     pendSol = _sentSolenoid;
        std::vector<uint8_t> pendLcd32 = _sentLcd32;
        bool commitPending = false;



        auto drainQueue = [&](){
            std::unique_lock<std::mutex> lk(_ioMx);
            _ioCv.wait_for(lk, std::chrono::milliseconds(5), [&]{ return !_ioQueue.empty() || !_ioRunning.load(); });
            if (!_ioRunning.load()) return;
            while(!_ioQueue.empty()) {
                IoCmd c = std::move(_ioQueue.front());
                _ioQueue.pop_front();
                lk.unlock();

                switch (c.type) {
                    case IoCmd::SetLed: {
                        const uint8_t idx = (c.a >= 0x03) ? (c.a - 0x03) : 0;
                        const uint32_t bit = (1u << idx);
                        if (c.b) pendLedBitmap |= bit; else pendLedBitmap &= ~bit;
                    } break;

                    case IoCmd::SetDimming: {
                        if (c.a < 3) pendDim[c.a] = c.b;
                    } break;

                    case IoCmd::SetATSolenoid: {
                        pendSol = (c.b != 0);
                    } break;

                    case IoCmd::LcdPayload: {
                        pendLcd32 = std::move(c.payload);
                    } break;
                }

                lk.lock();
            }
        };

        while(_ioRunning.load()) {
            drainQueue();
            if (!_ioRunning.load()) break;

            // === Envoi coalescé ===

            // 1) LEDs : ne pousse que les différences
            uint32_t diff = pendLedBitmap ^ _sentLedBitmap;
            if (diff) {
                for (uint8_t i = 0; i < 32; ++i) {
                    uint32_t bit = (1u << i);
                    if (diff & bit) {
                        uint8_t ledId = 0x03 + i;
                        bool on = (pendLedBitmap & bit) != 0;
                        transport::sendLed(static_cast<transport::DevicePtr>(this), ledId, on);
                    }
                }
                _sentLedBitmap = pendLedBitmap;
            }

            // 2) Dimming : par canal si changé
            for (uint8_t ch = 0; ch < 3; ++ch) {
                if (_sentDimming[ch] != pendDim[ch]) {
                    transport::sendDimming(static_cast<transport::DevicePtr>(this), ch, pendDim[ch]);
                    _sentDimming[ch] = pendDim[ch];
                }
            }

            // 3) Solénoïde : si changé
            if (_sentSolenoid != pendSol) {
                transport::sendATSolenoid(static_cast<transport::DevicePtr>(this), pendSol);
                _sentSolenoid = pendSol;
            }

            // 4) LCD : coalescing + rate-limit + séquence stricte
            {
                using clock = std::chrono::steady_clock;
                static auto lastTx = clock::now();
                const auto minPeriod = std::chrono::duration<double>(_minLcdPeriod);

                const bool havePend = !pendLcd32.empty();
                if (havePend) {
                    const bool changed = (pendLcd32 != _sentLcd32);
                    const auto now = clock::now();
                    const bool timeOk = ((now - lastTx) >= minPeriod);

                    if (changed && timeOk) {
                        // --- Séquence atomique + pacing obligatoire ---
                        auto* dev = static_cast<transport::DevicePtr>(this);

                        debug_force("[PAP3][LCD] TX begin (seq=%u)\n", _seq);

                        // PAYLOAD
                        transport::sendLcdPayload(dev, _seq, pendLcd32);
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));

                        // EMPTY #1
                        transport::sendLcdEmptyFrame(dev, _seq);
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));

                        // EMPTY #2
                        transport::sendLcdEmptyFrame(dev, _seq);
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));

                        // COMMIT
                        transport::sendLcdCommit(dev, _seq);

                        debug_force("[PAP3][LCD] TX end (seq=%u)\n", _seq);

                        _sentLcd32 = pendLcd32;
                        lastTx = now;
                    }
                    // Sinon : pas changé / rate-limit → on n’envoie rien (et surtout pas de commit seul)
                }
            }
        }
    }
};

void StartPap3Demo(PAP3Device* dev);


} // namespace pap3::device