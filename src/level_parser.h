#ifndef LEVELPARSER_H
#define LEVELPARSER_H

#include <raylib.h> 
#include <stdint.h>

#define TILE_VOID    0
#define TILE_FLOOR   1
#define TILE_SPAWN_1 2
#define TILE_SPAWN_2 3

typedef struct LevelData {
	uint16_t* Tiles;
	uint32_t LevelWidth; 
	uint16_t LevelHeight;
} LevelData;

void parse_level(const char* path, LevelData* data);

#endif