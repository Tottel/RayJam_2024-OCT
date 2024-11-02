#include "level_parser.h"

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 
#include <assert.h>

void parse_level(const char* path, LevelData* data) {
	char* levelTxtData = LoadFileText(path);  

	// We're going to do this in passes to avoid too much dynamic memory allocation and complicated loops
	// Get the max width and height first
	uint32_t width = 0;
	uint16_t height = 0;
	
	{
		uint32_t charCounter = 0;
		uint32_t widthCounter = 0;

		while (true) {
			if (levelTxtData[charCounter] == '\n') { 
				height += 1;

				if (widthCounter > width) {
					width = widthCounter;
				}

				widthCounter = 0; 
				charCounter += 1;

				continue;
			}

			if (levelTxtData[charCounter] == '\0') {
				height += 1;

				if (widthCounter > width) {
					width = widthCounter;
				}

				break;
			}

			widthCounter += 1; 
			charCounter += 1;
		}
	}

	data->LevelWidth = width; 
	data->LevelHeight = height;
	data->Tiles = RL_CALLOC(width * height, sizeof(uint16_t)); 

	int currX = 0;
	int currY = 0; 
	int charCounter = 0;

	while (true) {
		if (levelTxtData[charCounter] == '\n') {
			currX = 0;
			currY += 1;
			charCounter += 1;
			continue;
		}

		if (levelTxtData[charCounter] == '\0') {
			break;
		}

		switch (levelTxtData[charCounter]) {
		case '=': // floors
			data->Tiles[currX + (currY * width)] = TILE_FLOOR;
			break;
		case 'x': // walls
			data->Tiles[currX + (currY * width)] = TILE_PLATFORM;
			break;
		case '1': // player spawn 1
			data->Tiles[currX + (currY * width)] = TILE_SPAWN_1;
			break;
		case '2': // player spawn 2
			data->Tiles[currX + (currY * width)] = TILE_SPAWN_2;
			break;
		case 'O': // enemy
			data->Tiles[currX + (currY * width)] = TILE_ENEMY;
			break;
		case ']': // end portal 1
			data->Tiles[currX + (currY * width)] = TILE_PORTAL_1;
			break;
		case '}': // end portal 2
			data->Tiles[currX + (currY * width)] = TILE_PORTAL_2;
			break;
		case ' ': // void
			data->Tiles[currX + (currY * width)] = TILE_VOID;
			break;
		default:
			//assert(false);
			break;
		}

		currX += 1;
		charCounter += 1;
	}

	RL_FREE(levelTxtData);
}