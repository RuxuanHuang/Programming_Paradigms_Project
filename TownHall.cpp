#include "TownHall.h"

USING_NS_CC;

TownHall* TownHall::create(const std::string& buildingFile,
    const std::string& turfFile,
    float buildingScale)
{
    TownHall* ret = new (std::nothrow) TownHall();
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool TownHall::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    // 先调用父类的init
    if (!Building::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 设置大本营特有属性
    this->setBuildingName("Town Hall");  // 使用从Building继承的方法
    this->setLevel(1);                   // 使用从Building继承的方法

    // 大本营占用更多格子（假设6x6）
    this->setBuildingTileSize(4, 4);

    return true;
}