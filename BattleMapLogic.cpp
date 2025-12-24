#include "BattleMapLogic.h"

TileState g_map[MAP_SIZE][MAP_SIZE];

bool isWalkable(int tileX, int tileY)
{
    if (tileX < -HALF || tileX >= HALF ||
        tileY < -HALF || tileY >= HALF)
        return false;

    return g_map[tileX + HALF][tileY + HALF] == TileState::EMPTY;
}

void occupyTile(int tileX, int tileY)
{
    g_map[tileX + HALF][tileY + HALF] = TileState::BLOCK;
}

void clearTile(int tileX, int tileY)
{
    g_map[tileX + HALF][tileY + HALF] = TileState::EMPTY;
}
