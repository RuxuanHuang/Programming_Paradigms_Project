#include "cocos2d.h"
#include "ui/CocosGUI.h"
USING_NS_CC;

const float TILE_W = 65.55f;    // tile ¿í
const float TILE_H = 49.1625f;  // tile ¸ß
const int HALF = 21;            // µ±Ç°µØÍ¼ tile °ë¾¶
// µØÍ¼Í¼Æ¬Æ«ÒÆ½ÃÕý
const float offsetX = 51.0f;
const float offsetY = 156.0f;
// tile ¡ú map local
static Vec2 tileToMapLocal(Node* mapNode, float tileX, float tileY)
{
    Size mapSize = mapNode->getContentSize();
    Vec2 mapCenter(mapSize.width / 2 + offsetX,
        mapSize.height / 2 + offsetY);

    Vec2 centered;
    centered.x = (tileX - tileY) * (TILE_W / 2.0f);
    centered.y = (tileX + tileY) * (TILE_H / 2.0f);

    return mapCenter + centered;
}

// map local ¡ú tile
static Vec2 mapLocalToTile(Node* mapNode, const Vec2& mapLocal)
{
    Size mapSize = mapNode->getContentSize();
    Vec2 mapCenter(mapSize.width / 2 + offsetX,
        mapSize.height / 2 + offsetY);
    Vec2 centered = mapLocal - mapCenter;

    float tileX = (centered.x / (TILE_W / 2.0f) +
        centered.y / (TILE_H / 2.0f)) * 0.5f;

    float tileY = (centered.y / (TILE_H / 2.0f) -
        centered.x / (TILE_W / 2.0f)) * 0.5f;

    return Vec2(tileX, tileY);
}

