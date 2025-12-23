#include "Wall.h"

USING_NS_CC;

Wall* Wall::create(const std::string& buildingFile,
    bool isHownTown,
    const std::string& turfFile,
    float buildingScale)
{
    Wall* ret = new (std::nothrow) Wall();
    ret->setIsHownTown(isHownTown);
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Wall::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    // 1. 先调用父类的init
    if (!Building::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 2. 设置基本属性
    this->setBuildingName("Wall");
    this->setBuildingScale(0.85f);
    this->setBuildingSize(1);
    
    this->setMaxLevel(3);
    this->setCost(1000);



    int hpValues[] = { 100,200,400 };
    int upgradeCosts[] = { 1000,5000,0 };
    // 3. 设置等级信息
    for (int i = 1; i <= 3; i++) {
        _upgradeSprites[i] = levelInformation{ hpValues[i - 1], upgradeCosts[i - 1], "" };
        std::string spriteFile = StringUtils::format("Wall/Wall%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }
    return true;
}



