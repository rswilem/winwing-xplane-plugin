#include "segment-display.h"

#include <algorithm>
#include <map>

namespace SegmentDisplay {

    uint8_t getSegmentRepresentation(char c) {
        char upperC = std::toupper(c);
        switch (upperC) {
            case '0':
                return 0xFA;
            case '1':
                return 0x60;
            case '2':
                return 0xD6;
            case '3':
                return 0xF4;
            case '4':
                return 0x6C;
            case '5':
                return 0xBC;
            case '6':
                return 0xBE;
            case '7':
                return 0xE0;
            case '8':
                return 0xFE;
            case '9':
                return 0xFC;
            case 'A':
                return 0xEE;
            case 'B':
                return 0xFE;
            case 'C':
                return 0x9A;
            case 'D':
                return 0x76;
            case 'E':
                return 0x9E;
            case 'F':
                return 0x8E;
            case 'G':
                return 0xBE;
            case 'H':
                return 0x6E;
            case 'I':
                return 0x60;
            case 'J':
                return 0x70;
            case 'K':
                return 0x0E;
            case 'L':
                return 0x1A;
            case 'M':
                return 0xA6;
            case 'N':
                return 0x26;
            case 'O':
                return 0xFA;
            case 'P':
                return 0xCE;
            case 'Q':
                return 0xEC;
            case 'R':
                return 0x06;
            case 'S':
                return 0xBC;
            case 'T':
                return 0x1E;
            case 'U':
                return 0x7A;
            case 'V':
                return 0x32;
            case 'W':
                return 0x58;
            case 'X':
                return 0x6E;
            case 'Y':
                return 0x7C;
            case 'Z':
                return 0xD6;
            case '-':
                return 0x04;
            case '#':
                return 0x36;
            case '/':
                return 0x60;
            case '\\':
                return 0xA0;
            case ':':
                return 0x00;
            case ' ':
                return 0x00;
            default:
                return 0x00;
        }
    }

    uint8_t getAGPSegmentMask(char c) {
        char upperC = std::toupper(c);
        switch (upperC) {
                // Numbers
            case '0':
                return 0x3F; // 011 1111
            case '1':
                return 0x06; // 000 0110
            case '2':
                return 0x5B; // 101 1011
            case '3':
                return 0x4F; // 100 1111
            case '4':
                return 0x66; // 110 0110
            case '5':
                return 0x6D; // 110 1101
            case '6':
                return 0x7D; // 111 1101
            case '7':
                return 0x07; // 000 0111
            case '8':
                return 0x7F; // 111 1111
            case '9':
                return 0x6F; // 110 1111

            // Letters (Standard 7-Segment Approximations)
            case 'A':
                return 0x77; // 111 0111
            case 'B':
                return 0x7C; // 111 1100 (b)
            case 'C':
                return 0x39; // 011 1001
            case 'D':
                return 0x5E; // 101 1110 (d)
            case 'E':
                return 0x79; // 111 1001
            case 'F':
                return 0x71; // 111 0001
            case 'G':
                return 0x3D; // 011 1101
            case 'H':
                return 0x76; // 111 0110
            case 'I':
                return 0x30; // 011 0000
            case 'J':
                return 0x1E; // 001 1110
            case 'K':
                return 0x76; // 111 0110 (H - approx)
            case 'L':
                return 0x38; // 011 1000
            case 'M':
                return 0x54; // 101 0100 (n - approx)
            case 'N':
                return 0x54; // 101 0100 (n)
            case 'O':
                return 0x3F; // 011 1111 (0)
            case 'P':
                return 0x73; // 111 0011
            case 'Q':
                return 0x67; // 110 0111 (q)
            case 'R':
                return 0x50; // 101 0000 (r)
            case 'S':
                return 0x6D; // 110 1101 (5)
            case 'T':
                return 0x78; // 111 1000 (t)
            case 'U':
                return 0x3E; // 011 1110
            case 'V':
                return 0x1C; // 001 1100 (u - approx)
            case 'W':
                return 0x2A; // 010 1010 (approx) or 0x3E (U)
            case 'X':
                return 0x76; // 111 0110 (H - approx)
            case 'Y':
                return 0x6E; // 110 1110 (y)
            case 'Z':
                return 0x5B; // 101 1011 (2)

            // Symbols
            case ' ':
                return 0x00; // Blank
            case '-':
                return 0x40; // G segment only
            case '_':
                return 0x08; // D segment only
            default:
                return 0x00; // Default to blank for unknown chars
        }
    }

    uint8_t swapNibbles(uint8_t value) {
        return ((value & 0x0F) << 4) | ((value & 0xF0) >> 4);
    }

    std::string fixStringLength(const std::string &value, int length, char fillChar) {
        std::string result = value;
        if (result.length() > static_cast<size_t>(length)) {
            result = result.substr(result.length() - length);
        }
        while (result.length() < static_cast<size_t>(length)) {
            result = fillChar + result;
        }
        return result;
    }

    std::vector<uint8_t> encodeString(int numSegments, const std::string &str) {
        std::vector<uint8_t> data(numSegments, 0);

        for (int i = 0; i < std::min(numSegments, static_cast<int>(str.length())); i++) {
            data[numSegments - 1 - i] = getSegmentRepresentation(str[i]);
        }

        return data;
    }

    std::vector<uint8_t> encodeStringSwapped(int numSegments, const std::string &str) {
        std::vector<uint8_t> data = encodeString(numSegments, str);
        data.push_back(0); // Add extra byte

        // Fix weird segment mapping
        for (int i = 0; i < data.size(); i++) {
            data[i] = swapNibbles(data[i]);
        }

        for (int i = 0; i < data.size() - 1; i++) {
            data[numSegments - i] = (data[numSegments - i] & 0x0F) | (data[numSegments - 1 - i] & 0xF0);
            data[numSegments - 1 - i] = data[numSegments - 1 - i] & 0x0F;
        }

        return data;
    }

    std::vector<uint8_t> encodeStringEfis(int numSegments, const std::string &str) {
        std::vector<uint8_t> data = encodeString(numSegments, str);
        std::vector<uint8_t> result(numSegments, 0);

        // Fix weird segment mapping for EFIS displays
        for (int i = 0; i < data.size(); i++) {
            result[i] |= (data[i] & 0x08) ? 0x01 : 0; // Upper left -> bit 0
            result[i] |= (data[i] & 0x04) ? 0x02 : 0; // Middle -> bit 1
            result[i] |= (data[i] & 0x02) ? 0x04 : 0; // Lower left -> bit 2
            result[i] |= (data[i] & 0x10) ? 0x08 : 0; // Bottom -> bit 3
            result[i] |= (data[i] & 0x80) ? 0x10 : 0; // Top -> bit 4
            result[i] |= (data[i] & 0x40) ? 0x20 : 0; // Upper right -> bit 5
            result[i] |= (data[i] & 0x20) ? 0x40 : 0; // Lower right -> bit 6
            result[i] |= (data[i] & 0x01) ? 0x80 : 0; // Dot -> bit 7
        }

        return result;
    }

    std::vector<uint8_t> encodeStringAGP(int numSegments, const std::string &str) {
        // ===================================================================
        // AGP 7-SEGMENT BINARY ENCODING
        // ===================================================================
        // Based on PAP3-MCP protocol analysis, AGP uses binary 7-segment encoding
        // where each bit in the 16-bit value controls a specific segment.
        //
        // Similar to PAP3-MCP which uses segment order: Mid-TopL-BotL-Bot-BotR-TopR-Top
        // AGP appears to use a scrambled/position-dependent bit mapping.
        //
        // Each position gets 4 bytes: [low_byte, high_byte, 0x00, 0x00]
        // The 16-bit value (low | high<<8) encodes:
        //   - 7 segment bits (a,b,c,d,e,f,g)
        //   - Position/state flags (CHR active, display enable, etc.)
        //
        // Standard 7-segment layout:
        //      aaa
        //     f   b
        //      ggg
        //     e   c
        //      ddd
        std::vector<uint8_t> result;
        result.reserve(numSegments * 4);

        std::string paddedStr = str;
        if (paddedStr.length() > static_cast<size_t>(numSegments)) {
            paddedStr = paddedStr.substr(paddedStr.length() - numSegments);
        }
        while (paddedStr.length() < static_cast<size_t>(numSegments)) {
            paddedStr = " " + paddedStr;
        }

        // AGP position-specific encoding maps (reverse-engineered from device captures)
        // Format: 8 positions total
        // Positions 0-1: CHR display (chronometer, when active)
        // Positions 2-7: UTC time display (HHMMSS format)
        // Position-specific maps based on confirmed observations

        // ===================================================================
        // CONFIRMED MAPPINGS from packet captures (15:46:16, 15:47:08, 15:47:10)
        // ===================================================================

        // Position 2: Hours tens (0-2 for 24-hour format)
        // CONFIRMED: 1 = 0x03F0 or 0x03FF (context-dependent)
        static const std::map<char, uint16_t> pos2Map = {
            {'0', 0x03F0}, // Estimate based on pattern
            {'1', 0x03FF}, // CONFIRMED: 15:47:08, 15:47:10 (variant 0x03F0 at 15:46:16)
            {'2', 0x02F0}, // Estimate based on frequency
            {' ', 0x0000},
        };

        // Position 3: Hours ones (0-9)
        // CONFIRMED: 5 = 0x02A0 or 0x022F (context-dependent)
        static const std::map<char, uint16_t> pos3Map = {
            {'0', 0x03A0}, // Estimate
            {'1', 0x0320}, // Estimate
            {'2', 0x012F}, // Estimate
            {'3', 0x032F}, // Estimate
            {'4', 0x022F}, // Estimate
            {'5', 0x02A0}, // CONFIRMED: 15:46:16 (variant 0x022F at 15:47:08, 15:47:10)
            {'6', 0x02A0}, // Estimate (same as 5 - needs more data)
            {'7', 0x002F}, // Estimate
            {'8', 0x0227}, // Estimate
            {'9', 0x03A0}, // Estimate
            {' ', 0x0000},
        };

        // Position 4: Minutes tens (0-5 only)
        // CONFIRMED: 4 = 0x0280 or 0x0207 (context-dependent!)
        static const std::map<char, uint16_t> pos4Map = {
            {'0', 0x0080}, // Estimate
            {'1', 0x010F}, // Estimate
            {'2', 0x0207}, // Estimate
            {'3', 0x0307}, // Estimate
            {'4', 0x0280}, // CONFIRMED: 15:46:16 (variant 0x0207 at 15:47:08, 15:47:10)
            {'5', 0x0380}, // Estimate
            {' ', 0x0000},
        };

        // Position 5: Minutes ones (0-9)
        // CONFIRMED: 6 = 0x02E0, 7 = 0x026F
        static const std::map<char, uint16_t> pos5Map = {
            {'0', 0x00E0}, // Estimate
            {'1', 0x006F}, // Estimate
            {'2', 0x0167}, // Estimate
            {'3', 0x0367}, // Estimate
            {'4', 0x03E0}, // Estimate
            {'5', 0x026F}, // Estimate
            {'6', 0x02E0}, // CONFIRMED: 15:46:16
            {'7', 0x026F}, // CONFIRMED: 15:47:08, 15:47:10
            {'8', 0x01E0}, // Estimate
            {'9', 0x036F}, // Estimate
            {' ', 0x0000},
        };

        // Position 6: Seconds tens (0-5 only)
        // CRITICAL: 0 and 1 BOTH use 0x0068 when CHR is active!
        // When CHR OFF: digit 1 = 0x02E0
        static const std::map<char, uint16_t> pos6Map = {
            {'0', 0x0068}, // CONFIRMED: 15:47:08
            {'1', 0x0068}, // CONFIRMED: 15:47:10 (variant 0x02E0 at 15:46:16 when CHR OFF)
            {'2', 0x0260}, // Estimate
            {'3', 0x0268}, // Estimate
            {'4', 0x03E0}, // Estimate
            {'5', 0x02E0}, // Estimate
            {' ', 0x0000},
        };

        // Position 7: Seconds ones (0-9)
        // CRITICAL: Only CHR state bit changes! Digit NOT encoded here!
        // CHR OFF: 0x01E0, CHR ON: 0x01EC (difference = 0x0C bit)
        // CONFIRMED: 0, 6, 8 all share same encoding per CHR state
        static const std::map<char, uint16_t> pos7Map = {
            {'0', 0x01EC}, // CONFIRMED: 15:47:10 (CHR ON)
            {'1', 0x01EC}, // Assume CHR typically ON
            {'2', 0x01EC}, // Assume CHR typically ON
            {'3', 0x01EC}, // Assume CHR typically ON
            {'4', 0x01EC}, // Assume CHR typically ON
            {'5', 0x01EC}, // Assume CHR typically ON
            {'6', 0x01EC}, // Use 0x01E0 when CHR OFF (see 15:46:16)
            {'7', 0x01EC}, // Assume CHR typically ON
            {'8', 0x01EC}, // CONFIRMED: 15:47:08 (CHR ON)
            {'9', 0x01EC}, // Assume CHR typically ON
            {' ', 0x0000},
        };

        // Use confirmed empirical mappings (binary encoding is position-dependent)
        // TODO: Decode the exact bit-to-segment mapping once more packet data is available
        const std::map<char, uint16_t> *positionMaps[] = {
            &pos2Map, &pos3Map, &pos4Map, &pos5Map, &pos6Map, &pos7Map, &pos7Map, &pos7Map};

        for (int i = 0; i < numSegments; ++i) {
            char c = std::toupper(paddedStr[i]);
            uint16_t agpValue = 0x0000;

            // Use position-specific map if available
            if (i < 8 && positionMaps[i] != nullptr) {
                auto it = positionMaps[i]->find(c);
                if (it != positionMaps[i]->end()) {
                    agpValue = it->second;
                }
            }

            // AGP format: [low_byte, high_byte, 0x00, 0x00] per position
            result.push_back(agpValue & 0xFF);        // Low byte
            result.push_back((agpValue >> 8) & 0xFF); // High byte
            result.push_back(0x00);                   // Reserved
            result.push_back(0x00);                   // Reserved
        }

        return result;
    }
}
