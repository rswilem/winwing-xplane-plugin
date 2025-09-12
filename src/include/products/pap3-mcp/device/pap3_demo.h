// src/include/products/pap3-mcp/device/pap3_demo.h
#pragma once
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace pap3::device {

    class PAP3Device; // forward declaration

    class PAP3Demo {
        public:
            explicit PAP3Demo(PAP3Device *dev);
            ~PAP3Demo();

            void start(const std::vector<std::uint8_t> &ledIds,
                       float lcdFps = 12.0f,
                       float ledFps = 12.0f,
                       float hbFps = 24.0f);
            void stop();

            bool running() const noexcept {
                return _running;
            }

            // Public trampoline (X-Plane flight loop)
            static float FlightLoopThunk(float, float, int, void *refcon);

            // ---------- make these PUBLIC so pap3_demo.cpp can use them ----------
            // 7-seg addressing helpers used by the marquee implementation
            enum class SegGroupId : std::uint8_t { G1 = 0,
                                                   G2,
                                                   G3,
                                                   G4 };

            struct DigitSlot {
                    SegGroupId group;
                    std::uint8_t mask;
            };

            // --------------------------------------------------------------------

        private:
            // One-tick update called by flight loop
            float updateTick(float elapsedSinceLastCall);

            // --- LCD marquee ---
            void lcdStepMarquee();
            std::vector<std::uint8_t> buildLcdPayloadForWindow(int startCol) const;

            // --- LED chase ---
            void ledStepChase();

            // --- Heartbeat (backlight+LCD only) ---
            void heartbeatStep();

            // --- Helpers for 7-seg encoding ---
            static std::uint8_t segMaskForChar(char c);
            void orSeg(std::vector<std::uint8_t> &p, std::uint8_t absOffset, std::uint8_t bit) const;
            void lightDigit(std::vector<std::uint8_t> &p, const DigitSlot &slot, std::uint8_t pattern) const;

        private:
            PAP3Device *_dev{nullptr}; // non-owning
            bool _running{false};

            // Scheduling
            double _accumLcd{0.0}, _accumLed{0.0}, _accumHb{0.0};
            double _dtLcd{1.0 / 12.0}, _dtLed{1.0 / 12.0}, _dtHb{1.0 / 24.0};

            // LCD marquee state
            std::string _text{" HELLOoo  "}; // padded
            int _col{0};
            int _lineLen{12};

            // LED chase state
            std::vector<std::uint8_t> _ledIds;
            std::size_t _ledIndex{0};
            std::mt19937 _rng;

            // Heartbeat state
            double _phase{0.0}; // 0..1
            std::uint8_t _hbMin{40};
            std::uint8_t _hbMax{220};
    };

}