#pragma once
#include"MapTools.h"
static const int MAP_SIZE = BATTLE_MAP_HALF * 2;
enum class TileState
{
    EMPTY,
    BLOCK
};

extern TileState g_map[MAP_SIZE][MAP_SIZE];

bool isWalkable(int tileX, int tileY);
void occupyTile(int tileX, int tileY);
void clearTile(int tileX, int tileY);
