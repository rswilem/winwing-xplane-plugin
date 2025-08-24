#ifndef FONT_H
#define FONT_H

#include <vector>

enum class FontVariant : unsigned char {
    FontAirbus1,
    FontAirbus2,
    FontAirbus3,
    Font737,
    FontXCrafts,
    FontVGA1,
    FontVGA2,
    FontVGA3,
    FontVGA4,
};

class Font {
public:
    static const std::vector<std::vector<unsigned char>> GlyphData(FontVariant variant, unsigned char hardwareIdentifier);
};
    
#endif
