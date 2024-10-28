#include "image_color_parser.h"

Texture load_and_convert_texture(const char* path, Color* allowed_colors, uint8_t color_count) {
	Image temp = load_and_convert_image(path, allowed_colors, color_count);

	Texture texture = LoadTextureFromImage(temp);

	UnloadImage(temp);

	return texture;
}

Image load_and_convert_image(const char* path, Color* allowed_colors, uint8_t color_count) {
	Image temp = LoadImage(path);

	for (int y = 0; y < temp.height; y++) {
		for (int x = 0; x < temp.width; x++) {
			Color color = GetImageColor(temp, x, y);

			if (color.a == 0) continue;

			float bestDiffValue = 999999.9f;
			uint8_t bestColorIndex = 0;
			for (uint8_t c = 0; c < color_count; c++) {
				Color testColor = allowed_colors[c];

				float diffValue = ((color.r - testColor.r) * (color.r - testColor.r))
					+ ((color.g - testColor.g) * (color.g - testColor.g))
					+ ((color.b - testColor.b) * (color.b - testColor.b));

				if (diffValue < bestDiffValue) {
					bestDiffValue = diffValue;
					bestColorIndex = c;
				}
			}

			ImageDrawPixel(&temp, x, y, allowed_colors[bestColorIndex]);
		}
	}

	return temp;
}