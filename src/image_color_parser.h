#ifndef IMAGECOLORPARSER_H
#define IMAGECOLORPARSER_H

#include <raylib.h>
#include <stdint.h>

Texture load_and_convert_texture(const char* path, Color* allowed_colors, uint8_t color_count);
Image load_and_convert_image(const char* path, Color* allowed_colors, uint8_t color_count);
#endif