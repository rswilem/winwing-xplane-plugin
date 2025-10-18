#include "font.h"

#include "737.h"
#include "airbus.h"
#include "default.h"
#include "md11-cdu.h"
#include "vga_1.h"
#include "vga_2.h"
#include "vga_3.h"
#include "vga_4.h"
#include "xcrafts.h"

#include <cstddef>

const std::vector<std::vector<unsigned char>> Font::GlyphData(FontVariant variant, unsigned char hardwareIdentifier) {
    std::vector<std::vector<unsigned char>> result = {};

    switch (variant) {
        case FontVariant::FontAirbus:
            result = fmcFontAirbus;
            break;

        case FontVariant::Font737:
            result = fmcFont737;
            break;

        case FontVariant::FontXCrafts:
            result = fmcFontXCrafts;
            break;

        case FontVariant::FontVGA1:
            result = fmcFontVGA1;
            break;

        case FontVariant::FontVGA2:
            result = fmcFontVGA2;
            break;

        case FontVariant::FontVGA3:
            result = fmcFontVGA3;
            break;

        case FontVariant::FontVGA4:
            result = fmcFontVGA4;
            break;

        case FontVariant::FontMD11:
            result = fmcFontMd11Cdu;
            break;

        case FontVariant::Default:
        default:
            result = fmcFontDefault;
            break;
    }

    for (auto &row : result) {
        for (size_t i = 0; i + 1 < row.size(); i++) {
            if (row[i] == 0x32 && row[i + 1] == 0xbb) { // Sniffed packets always have the MCDU identifier
                row[i] = hardwareIdentifier;
                row[i + 1] = 0xbb;
            }
        }
    }
    return result;
}
