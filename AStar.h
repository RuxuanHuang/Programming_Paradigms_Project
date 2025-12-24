#pragma once
#include <vector>
#include "cocos2d.h"

std::vector<cocos2d::Vec2> findPath(
    const cocos2d::Vec2& startTile,
    const cocos2d::Vec2& endTile
);
