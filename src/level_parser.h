#ifndef LEVELPARSER_H
#define LEVELPARSER_H

#include <raylib.h>
#include <stdint.h>

// Tile Types:
// 0 = void
// 1 = floor
// 2 = player 1 spawn 
// 3 = player 2 spawn

typedef struct LevelData {
	uint16_t* Tiles;
	uint32_t LevelWidth;
	uint16_t LevelHeight;
} LevelData;

void parse_level(const char* path, LevelData* data);

#endif