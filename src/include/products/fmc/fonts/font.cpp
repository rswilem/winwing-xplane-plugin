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

const std::vector<std::vector<unsigned char>> Font::GlyphData(std::string filename, unsigned char hardwareIdentifier) {
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

    convertGlyphDataForHardware(result, hardwareIdentifier);

    return result;
}

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

        case FontVariant::FontMD11:
            result = fmcFontMd11Cdu;
            break;

        case FontVariant::Default:
        default:
            result = fmcFontDefault;
            break;
    }

    convertGlyphDataForHardware(result, hardwareIdentifier);

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

void Font::convertGlyphDataForHardware(std::vector<std::vector<unsigned char>> &data, unsigned char hardwareIdentifier) {
    for (auto &row : data) {
        for (size_t i = 0; i + 1 < row.size(); i++) {
            if (row[i] == 0x32 && row[i + 1] == 0xbb) { // Sniffed packets always have the MCDU identifier
                row[i] = hardwareIdentifier;
                row[i + 1] = 0xbb;
            }
        }
    }
}
