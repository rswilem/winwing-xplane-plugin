#include "compose.h"

#include <algorithm>
#include <cmath>

namespace seg = pap3::lcd::segments;

namespace pap3::lcd::compose {

    // ---- helpers ----------------------------------------------------------

    static inline void digits4(int value, int &k, int &h, int &t, int &u) {
        value = std::clamp(value, 0, 9999);
        k = (value / 1000) % 10;
        h = (value / 100) % 10;
        t = (value / 10) % 10;
        u = value % 10;
    }

    static inline void digits3(int value, int &h, int &t, int &u) {
        value = std::clamp(value, 0, 999);
        h = (value / 100) % 10;
        t = (value / 10) % 10;
        u = value % 10;
    }

    static inline void digits5(int value, int &t10k, int &k, int &h, int &t, int &u) {
        value = std::clamp(value, 0, 99999);
        t10k = (value / 10000) % 10;
        k = (value / 1000) % 10;
        h = (value / 100) % 10;
        t = (value / 10) % 10;
        u = value % 10;
    }

    // ---- build ------------------------------------------------------------

    Payload build(const Snapshot &s) {
        seg::Payload p;
        seg::clear(p);

        auto drawMaybe = [&](auto group, auto flag, int val, bool visible) {
            if (visible) {
                seg::drawDigit(group, p, flag, val);
            }
        };

        // SPD: IAS vs. MACH rendering rules with dual-format MACH support.
        // - If spd >= 100      => IAS (knots). Round half-up. Show K/H/T/U with K/H leading-zero suppression. No dot. IAS label.
        // - If 1.0f <= spd < 100 => MACH given as Mach×100 (e.g. 78 == M0.78).
        // - If spd < 1.0f      => MACH given as real fraction (e.g. 0.6454).
        // In MACH: show only the two decimals ".xx" mapped to T/U, round using the 3rd decimal. MACH label + decimal dot.
        {
            const float spd = s.spd;

            auto round_half_up_to_int = [](float x) -> int {
                return static_cast<int>(std::floor(x + 0.5f));
            };

            const bool isMach = (spd < 100.0f); // both real Mach (<1) and Mach×100 (<100)

            if (isMach) {
                // Normalize to a 0..1 Mach fraction.
                float mach = 0.0f;
                if (spd < 1.0f) {
                    // spd already is Mach (e.g. 0.6454)
                    mach = std::clamp(spd, 0.0f, 0.9999f);
                } else {
                    // spd is Mach×100 (e.g. 78 -> 0.78)
                    mach = std::clamp(spd / 100.0f, 0.0f, 0.9999f);
                }

                // Take two decimals only, rounded with the 3rd decimal.
                // ex: 0.786 -> 78 + (6>=5 ? +1 : +0) -> 79
                const int twoDigits = std::clamp(round_half_up_to_int(mach * 1000.0f / 10.0f), 0, 99);
                const int tens = (twoDigits / 10) % 10;
                const int units = twoDigits % 10;

                // Draw only T and U; K and H stay blank.
                seg::drawDigit(seg::G0, p, seg::SPD_TENS, tens);
                seg::drawDigit(seg::G0, p, seg::SPD_UNITS, units);

                // Labels and indicators
                seg::setFlag(p, seg::OFF_36, seg::LBL_IAS, false);
                seg::setFlag(p, seg::OFF_32, seg::LBL_MACH_L, true && s.lblIAS);
                seg::setFlag(p, seg::OFF_2E, seg::LBL_MACH_R, true && s.lblIAS);

                // Bars unchanged
                seg::setFlag(p, seg::OFF_22, seg::SPD_BAR_TOP, s.spdBarTop);
                seg::setFlag(p, seg::OFF_1E, seg::SPD_BAR_BOTTOM, s.spdBarBot);

                // Decimal dot ON in MACH mode
                seg::setFlag(p, seg::OFF_19, seg::DOT_SPD, true);

            } else {
                // IAS mode: integer knots, round half-up.
                const int ias = std::max(0, round_half_up_to_int(spd));
                int k, h, t, u;
                digits4(ias, k, h, t, u);

                const bool showK = (k != 0);
                const bool showH = showK || (h != 0);

                drawMaybe(seg::G0, seg::SPD_HUNDREDS, h, showH);
                seg::drawDigit(seg::G0, p, seg::SPD_TENS, t);
                seg::drawDigit(seg::G0, p, seg::SPD_UNITS, u);

                // Labels and indicators
                seg::setFlag(p, seg::OFF_36, seg::LBL_IAS, true && s.lblIAS);
                seg::setFlag(p, seg::OFF_32, seg::LBL_MACH_L, false);
                seg::setFlag(p, seg::OFF_2E, seg::LBL_MACH_R, false);

                // Bars unchanged
                seg::setFlag(p, seg::OFF_22, seg::SPD_BAR_TOP, s.spdBarTop);
                seg::setFlag(p, seg::OFF_1E, seg::SPD_BAR_BOTTOM, s.spdBarBot);

                // No decimal dot in IAS mode
                seg::setFlag(p, seg::OFF_19, seg::DOT_SPD, false);
            }

            if (!isMach) {
                // Special MCP "A" and "8" digits in IAS mode only
                if (s.digitA) {
                    seg::drawLetterA(seg::G0, p, seg::SPD_KILO);
                }
                if (s.digitB) {
                    seg::drawDigit(seg::G0, p, seg::SPD_KILO, 8);
                }
            }
        }

        // CAPT CRS: 3 digits -> G0 with flags [H,T,U] = [0x80,0x40,0x20]
        {
            int h, t, u;
            digits3(std::max(0, s.crsCapt), h, t, u);
            seg::drawDigit(seg::G0, p, seg::CPT_CRS_HUNDREDS, h);
            seg::drawDigit(seg::G0, p, seg::CPT_CRS_TENS, t);
            seg::drawDigit(seg::G0, p, seg::CPT_CRS_UNITS, u);

            // Final dot for CAPT_CRS
            seg::setFlag(p, seg::OFF_19, seg::DOT_CPT_CRS, s.dotCrsCapt);
        }

        // HDG: 3 digits -> G1 with flags [H,T,U] = [0x40,0x20,0x10]
        {
            int h, t, u;
            // Clamp 0..359 but still draw as 3 digits (000..359)
            int hdg = std::clamp(s.hdg, 0, 359);
            digits3(hdg, h, t, u);
            seg::drawDigit(seg::G1, p, seg::HDG_HUNDREDS, h);
            seg::drawDigit(seg::G1, p, seg::HDG_TENS, t);
            seg::drawDigit(seg::G1, p, seg::HDG_UNITS, u);

            // Final dot for HDG
            seg::setFlag(p, seg::OFF_26, seg::DOT_HDG, s.dotHdg);

            // HDG/TRK labels (two halves each)
            seg::setFlag(p, seg::OFF_36, seg::LBL_HDG_L, s.lblHDG);
            seg::setFlag(p, seg::OFF_32, seg::LBL_HDG_R, s.lblHDG);
            seg::setFlag(p, seg::OFF_2E, seg::LBL_TRK_L, s.lblTRK);
            seg::setFlag(p, seg::OFF_2A, seg::LBL_TRK_R, s.lblTRK);
        }

        // ALT: 5 digits
        //   High 3 -> G1 flags [ALT_TENS_KILO, ALT_KILO, ALT_HUNDREDS] = [0x04,0x02,0x01]
        //   Low  2 -> G2 flags [ALT_TENS, ALT_UNITS] = [0x80,0x40]
        {
            int d10k, dk, dh, dt, du;
            digits5(std::max(0, s.alt), d10k, dk, dh, dt, du);

            // Visibility rules:
            // - Show 10k only if non-zero.
            // - If 10k is shown, always show the kilo digit (even if it is 0) to keep a fixed 5-digit look like "40300".
            // - If 10k is hidden, show kilo only if it is non-zero OR any lower digit is non-zero (single leading zero suppression).
            const bool anyLower = (dk != 0) || (dh != 0) || (dt != 0) || (du != 0);
            const bool show10k = (d10k != 0);

            // High 3 on G1
            drawMaybe(seg::G1, seg::ALT_TENS_KILO, d10k, show10k);
            seg::drawDigit(seg::G1, p, seg::ALT_KILO, dk);
            seg::drawDigit(seg::G1, p, seg::ALT_HUNDREDS, dh); // hundreds always shown (0..9)

            // Low 2 on G2
            seg::drawDigit(seg::G2, p, seg::ALT_TENS, dt);
            seg::drawDigit(seg::G2, p, seg::ALT_UNITS, du);

            seg::setFlag(p, seg::OFF_1A, seg::DOT_ALT, s.dotAlt);
        }

        // VVI: sign + 4 digits on G2 [K,H,T,U] = [0x08,0x04,0x02,0x01]
        {
            if (s.showVvi) {
                const int v = s.vvi;
                const int absV = std::clamp(std::abs(v), 0, 9999);
                int k, h, t, u;
                digits4(absV, k, h, t, u);

                if (absV >= 1000) {
                    seg::drawDigit(seg::G2, p, seg::VSPD_KILO, k);
                }
                if (absV >= 100) {
                    seg::drawDigit(seg::G2, p, seg::VSPD_HUNDREDS, h);
                }
                if (absV >= 10) {
                    seg::drawDigit(seg::G2, p, seg::VSPD_TENS, t);
                }
                if (absV >= 1) {
                    seg::drawDigit(seg::G2, p, seg::VSPD_UNITS, u);
                }

                // VVI sign and dot
                const bool neg = (v < 0);
                const bool pos = (v > 0);
                seg::setFlag(p, seg::OFF_1F, seg::VSPD_MINUS, neg || pos);
                seg::setFlag(p, seg::OFF_2C, seg::VSPD_PLUS_TOP, pos);
                seg::setFlag(p, seg::OFF_28, seg::VSPD_PLUS_BOT, pos);
                seg::setFlag(p, seg::OFF_1B, seg::DOT_VSPD, s.dotVvi);

                // V/S vs FPA label
                if (absV >= 1) {
                    seg::setFlag(p, seg::OFF_38, seg::LBL_VS, s.lblVS);
                }
                seg::setFlag(p, seg::OFF_34, seg::LBL_FPA, s.lblFPA);
            }
            // When showVvi is false, the segment payload stays cleared by seg::clear(p).
        }

        // FO CRS: 3 digits -> G3 with flags [H,T,U] = [0x40,0x20,0x10]
        {
            int h, t, u;
            digits3(std::max(0, s.crsFo), h, t, u);
            seg::drawDigit(seg::G3, p, seg::FO_CRS_HUNDREDS, h);
            seg::drawDigit(seg::G3, p, seg::FO_CRS_TENS, t);
            seg::drawDigit(seg::G3, p, seg::FO_CRS_UNITS, u);

            // Final dot for FO_CRS
            seg::setFlag(p, seg::OFF_1C, seg::DOT_FO_CRS, s.dotCrsFo);
        }

        return p;
    }

} // namespace pap3::lcd::compose
