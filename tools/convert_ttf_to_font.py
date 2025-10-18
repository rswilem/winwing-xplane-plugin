#!/usr/bin/env python3
"""
Convert TTF font to Winwing FMC font format

This script converts a TTF font file to the C++ header format used by the Winwing plugin.
The format appears to be glyph bitmap data sent to the hardware display.

Usage:
    python3 convert_ttf_to_font.py MD-11-CDU.ttf md11-cdu.h
"""

import sys
from pathlib import Path

try:
    from PIL import Image, ImageFont, ImageDraw
except ImportError:
    print("Error: PIL (Pillow) is required. Install with: pip3 install Pillow")
    sys.exit(1)

# Font rendering parameters
FONT_SIZE = 24  # Adjust based on your display needs
GLYPH_WIDTH = 24  # Width of each character in pixels
GLYPH_HEIGHT = 32  # Height of each character in pixels

# Characters to include in the font (ASCII 32-126 plus extended)
CHAR_RANGE = list(range(32, 127))  # Standard ASCII printable

def render_glyph(font, char):
    """
    Render a single character to a bitmap and convert to byte array.
    """
    # Create an image for the glyph
    img = Image.new('1', (GLYPH_WIDTH, GLYPH_HEIGHT), color=0)  # 1-bit black & white
    draw = ImageDraw.Draw(img)
    
    # Draw the character
    try:
        draw.text((0, 0), char, font=font, fill=1)
    except:
        # If character can't be rendered, leave it blank
        pass
    
    # Convert to bytes
    pixels = list(img.getdata())
    
    # Pack pixels into bytes (8 pixels per byte)
    byte_data = []
    for y in range(GLYPH_HEIGHT):
        for x in range(0, GLYPH_WIDTH, 8):
            byte_val = 0
            for bit in range(8):
                if x + bit < GLYPH_WIDTH:
                    pixel_index = y * GLYPH_WIDTH + x + bit
                    if pixel_index < len(pixels) and pixels[pixel_index]:
                        byte_val |= (1 << (7 - bit))
            byte_data.append(byte_val)
    
    # Add header bytes (based on observed format from other fonts)
    # Format: 0xF0, 0x00, char_index, 0x3C, ...glyph_data...
    char_code = ord(char) if isinstance(char, str) else char
    header = [0xF0, 0x00, char_code & 0xFF, 0x3C]
    
    return header + byte_data

def convert_font(ttf_path, output_h_path):
    """
    Convert TTF font to C++ header file.
    """
    print(f"Loading font from {ttf_path}...")
    
    try:
        font = ImageFont.truetype(ttf_path, FONT_SIZE)
    except Exception as e:
        print(f"Error loading font: {e}")
        return False
    
    print(f"Rendering {len(CHAR_RANGE)} glyphs...")
    
    glyphs = []
    for char_code in CHAR_RANGE:
        char = chr(char_code)
        glyph_data = render_glyph(font, char)
        glyphs.append(glyph_data)
    
    # Generate C++ header file
    var_name = Path(output_h_path).stem.replace('-', '_').replace('.', '_')
    guard_name = var_name.upper() + "_H"
    
    print(f"Writing C++ header to {output_h_path}...")
    
    with open(output_h_path, 'w') as f:
        f.write(f"#ifndef {guard_name}\n")
        f.write(f"#define {guard_name}\n")
        f.write("#include <vector>\n\n")
        f.write(f"const std::vector<std::vector<unsigned char>> fmcFont{var_name.title().replace('_', '')} = {{\n")
        
        for i, glyph in enumerate(glyphs):
            f.write("    {")
            f.write(", ".join(f"0x{byte:02X}" for byte in glyph))
            f.write("}")
            if i < len(glyphs) - 1:
                f.write(",\n")
            else:
                f.write("\n")
        
        f.write("};\n\n")
        f.write(f"#endif\n")
    
    print(f"Done! Font converted successfully.")
    print(f"  - {len(glyphs)} glyphs generated")
    print(f"  - Output: {output_h_path}")
    print(f"\nNext steps:")
    print(f"  1. Add to font.h: enum class FontVariant {{ ..., FontMD11 }};")
    print(f"  2. Add to font.cpp: #include \"{Path(output_h_path).name}\"")
    print(f"  3. Add to font.cpp switch: case FontVariant::FontMD11: result = fmcFont{var_name.title().replace('_', '')}; break;")
    print(f"  4. Update rotatemd11-fmc-profile.cpp: product->setFont(Font::GlyphData(FontVariant::FontMD11, product->identifierByte));")
    
    return True

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 convert_ttf_to_font.py <input.ttf> <output.h>")
        print("Example: python3 convert_ttf_to_font.py MD-11-CDU.ttf md11-cdu.h")
        sys.exit(1)
    
    ttf_path = sys.argv[1]
    output_h_path = sys.argv[2]
    
    if not Path(ttf_path).exists():
        print(f"Error: Input file '{ttf_path}' not found")
        sys.exit(1)
    
    success = convert_font(ttf_path, output_h_path)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()

