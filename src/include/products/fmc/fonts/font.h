#ifndef FONT_H
#define FONT_H

#include <vector>

enum class FontVariant : unsigned char {
    Default,
    FontAirbus,
    Font737,
    FontXCrafts,
    FontVGA1,
    FontVGA2,
    FontVGA3,
    FontVGA4,
    FontMD11,
};

class Font {
    public:
        static const std::vector<std::vector<unsigned char>> GlyphData(FontVariant variant, unsigned char hardwareIdentifier);
};

#endif
