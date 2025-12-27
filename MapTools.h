#include "cocos2d.h"
#include "ui/CocosGUI.h"
USING_NS_CC;

const float TILE_W = 39.33f;    // tile ¿í
const float TILE_H = 29.4975f;  // tile ¸ß

static float BATTLE_MAP_TILE_W = 31.5f;    // tile ¿í
static float BATTLE_MAP_TILE_H = 23.625f;  // tile ¸ß

const int HALF = 21;            // µ±Ç°µØÍ¼ tile °ë¾¶
// µØÍ¼Í¼Æ¬Æ«ÒÆ½ÃÕý
static float offsetX = 31.0f;
static float offsetY = 92.0f;

static float BATTLE_MAP_OFFSETX = -1.0f;    
static float BATTLE_MAP_OFFSETY = 69.0f;  
const int BATTLE_MAP_HALF = 18;
// tile ¡ú map local
static Vec2 tileToMapLocal(Node* mapNode, float tileX, float tileY,bool isHomeTown)
{
    Size mapSize = mapNode->getContentSize();
	Vec2 mapCenter;
    if (isHomeTown) {
        mapCenter = { mapSize.width / 2 + offsetX,mapSize.height / 2 + offsetY };
        
    }
    else {
        mapCenter = { mapSize.width / 2 + BATTLE_MAP_OFFSETX,
            mapSize.height / 2 + BATTLE_MAP_OFFSETY };
    }
    

    Vec2 centered;
    if (isHomeTown) {
        centered.x = (tileX - tileY) * (TILE_W / 2.0f);
        centered.y = (tileX + tileY) * (TILE_H / 2.0f);
    }
    else {
        centered.x = (tileX - tileY) * (BATTLE_MAP_TILE_W / 2.0f);
        centered.y = (tileX + tileY) * (BATTLE_MAP_TILE_H / 2.0f);
    }
    

    return mapCenter + centered;
}

// map local ¡ú tile
static Vec2 mapLocalToTile(Node* mapNode, const Vec2& mapLocal,  bool isHomeTown)
{
    Size mapSize = mapNode->getContentSize();
    Vec2 mapCenter;
    if (isHomeTown) {
        mapCenter = { mapSize.width / 2 + offsetX,mapSize.height / 2 + offsetY };

    }
    else {
        mapCenter = { mapSize.width / 2 + BATTLE_MAP_OFFSETX,
            mapSize.height / 2 + BATTLE_MAP_OFFSETY };
    }
    Vec2 centered = mapLocal - mapCenter;

    float tileX = 0;
    float tileY = 0;
	if (isHomeTown) {
        tileX = (centered.x / (TILE_W / 2.0f) +
            centered.y / (TILE_H / 2.0f)) * 0.5f;

        tileY = (centered.y / (TILE_H / 2.0f) -
            centered.x / (TILE_W / 2.0f)) * 0.5f;
    } 
    else {
        tileX = (centered.x / (BATTLE_MAP_TILE_W / 2.0f) +
            centered.y / (BATTLE_MAP_TILE_H / 2.0f)) * 0.5f;

        tileY = (centered.y / (BATTLE_MAP_TILE_H / 2.0f) -
            centered.x / (BATTLE_MAP_TILE_W / 2.0f)) * 0.5f;
    }
    

    return Vec2(tileX, tileY);
}

