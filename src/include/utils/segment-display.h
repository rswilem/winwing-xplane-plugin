#ifndef SEGMENT_DISPLAY_H
#define SEGMENT_DISPLAY_H

#include <cstdint>
#include <string>
#include <vector>

namespace SegmentDisplay {

    // Get 7-segment representation for a character
    uint8_t getSegmentRepresentation(char c);

    uint8_t getAGPSegmentMask(char c);

    // Basic 7-segment string encoding (right-aligned)
    std::vector<uint8_t> encodeString(int numSegments, const std::string &str);

    // Swapped nibble encoding (for some displays)
    std::vector<uint8_t> encodeStringSwapped(int numSegments, const std::string &str);

    // EFIS-specific bit mapping
    std::vector<uint8_t> encodeStringEfis(int numSegments, const std::string &str);

    // Fix string length with leading zeros
    std::string fixStringLength(const std::string &value, int length);

    // Swap nibbles in a byte
    uint8_t swapNibbles(uint8_t value);

    // AGP 2-byte per digit encoding (little-endian format)
    std::vector<uint8_t> encodeStringAGP(int numSegments, const std::string &str);

}

#endif
