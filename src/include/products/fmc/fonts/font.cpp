#include "font.h"

#include "737.h"
#include "airbus.h"
#include "appstate.h"
#include "config.h"
#include "default.h"
#include "md11-cdu.h"
#include "vga_1.h"
#include "xcrafts.h"

#include <cstddef>
#include <dirent.h>
#include <XPLMUtilities.h>

const std::vector<std::vector<unsigned char>> Font::GlyphData(std::string filename, unsigned char hardwareIdentifier, FMCHardwareType hardwareType) {
    std::string fontFile = AppState::getInstance()->getPluginDirectory() + "/fonts/" + filename;
    FILE *file = fopen(fontFile.c_str(), "rb");
    if (file == NULL) {
        debug_force("Could not open custom font file: %s\n", fontFile.c_str());
        return {};
    }

    std::vector<std::vector<unsigned char>> result = {};
    while (!feof(file)) {
        unsigned char lengthByte = 0;
        size_t bytesRead = fread(&lengthByte, sizeof(unsigned char), 1, file);
        if (feof(file) || bytesRead == 0 || lengthByte == 0) {
            break;
        }

        std::vector<unsigned char> glyphData(lengthByte, 0);
        size_t dataRead = fread(glyphData.data(), sizeof(unsigned char), lengthByte, file);
        if (dataRead > 0) {
            result.push_back(glyphData);
        }
    }
    fclose(file);

    convertGlyphDataForHardware(result, hardwareIdentifier, hardwareType);

    return result;
}

const std::vector<std::vector<unsigned char>> Font::GlyphData(FontVariant variant, unsigned char hardwareIdentifier, FMCHardwareType hardwareType) {
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

        case FontVariant::FontMD11:
            result = fmcFontMd11Cdu;
            break;

        case FontVariant::Default:
        default:
            result = fmcFontDefault;
            break;
    }

    convertGlyphDataForHardware(result, hardwareIdentifier, hardwareType);

    return result;
}

const std::vector<std::string> Font::ReadCustomFontFiles() {
    std::vector<std::string> fontFiles;

    std::string fontsDirectory = AppState::getInstance()->getPluginDirectory() + "/fonts";

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(fontsDirectory.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fileName = ent->d_name;
            if (fileName.ends_with(".xpwwf")) {
                fontFiles.push_back(fileName);
            }
        }
        closedir(dir);
    } else {
        debug_force("Could not open fonts directory: %s\n", fontsDirectory.c_str());
    }

    return fontFiles;
}

const bool Font::IsCustomFontAvailable(std::string filename) {
    std::vector<std::string> availableFonts = ReadCustomFontFiles();
    for (const auto &availableFont : availableFonts) {
        if (availableFont == filename) {
            return true;
        }
    }
    return false;
}

void Font::convertGlyphDataForHardware(std::vector<std::vector<unsigned char>> &data, unsigned char hardwareIdentifier, FMCHardwareType hardwareType) {
    for (auto &row : data) {
        for (size_t i = 0; i + 1 < row.size(); i++) {
            if (row[i] == 0x32 && row[i + 1] == 0xbb) { // Sniffed packets always have the MCDU identifier
                row[i] = hardwareIdentifier;
                row[i + 1] = 0xbb;
            }
        }

        if (row.size() > 23 && row[0] == 0xF0 && row[1] == 0x00 && row[17] == 0x08 && row[18] == 0x00 && row[21] == 0x34 && row[23] == 0x25) {
            uint8_t leftOffset = 16; // MCDU std 0x34 (dec 52, sap 16) | PFP std 0x32 (dec 50, sap 14)
            uint8_t topOffset = 17;  // MCDU std 0x25 (dec 37, sap 17) | PFP std 0x20 (dec 32, sap 12)
            
            if (hardwareType == FMCHardwareType::HARDWARE_MCDU) {
                // MCDU defaults Left 0x34, Top 0x25 (SAP L16, T17)
                leftOffset = 16;
                topOffset = 17;
            }
            else {
                // PFP defaults Left 0x32, Top 0x20 (SAP L14, T12)
                leftOffset = 14;
                topOffset = 12;
            }

            row[21] = 36 + leftOffset;
            row[23] = 20 + topOffset;
        }
    }

    /*mcdu: top bottom 17, left right 16 char w/h 23/29
pfp: top bottom 12, left right 14, char w/h 23/32
f0 00 xx 2a 32 bb 00 00 18 01 00 00 AA AA AA 00 00 08 00 00 00 35 00 26 00 0e (top 18, left 17) (35 hex is 53, 26 hex is 38)
f0 00 xx 2a 32 bb 00 00 18 01 00 00 AA AA AA 00 00 08 00 00 00 34 00 25 00 0e (top 17, left 16) (34 hex is 52, 25 hex is 37)
f0 00 xx 2a 32 bb 00 00 18 01 00 00 AA AA AA 00 00 08 00 00 00 34 00 1c 00 0e (top 8, left 16) (34 hex is 52, 1c hex is 28)*/
    //{0xF0, 0x00, 0xCD, 0x2A, 0x32, 0xBB, 0x00, 0x00, 0x18, 0x01, 0x00, 0x00, 0x51, 0x82, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x34, 0x00, 0x25, 0x00, 0x0E, 0x00, 0x18, 0x00, 0x32, 0xBB, 0x00, 0x00, 0x05, 0x01, 0x00, 0x00, 0x51, 0x82, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
}
