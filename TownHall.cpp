#include "TownHall.h"

USING_NS_CC;

TownHall* TownHall::create(const std::string& buildingFile,
    bool isHownTown,
    const std::string& turfFile,
    float buildingScale)
{
    TownHall* ret = new (std::nothrow) TownHall();
    ret->setIsHownTown(isHownTown);
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
   
    // 1. 先调用父类的init
    if (!Building::init(buildingFile,turfFile, buildingScale)) {
        return false;
    }

    // 2. 设置大本营基本属性
    this->setBuildingName("Town Hall");
    this->setBuildingSize(4);
    this->setCost(1000);
    this->setHP(400);


    int hpValues[] = { 400,800,1600 };
	int upgradeCosts[] = { 1000,4000,0 };
    // 3. 设置大本营等级信息
    for (int i = 1; i <= 3; i++) {
		_upgradeSprites[i] = levelInformation{ hpValues[i - 1], upgradeCosts[i - 1], "" };
        std::string spriteFile = StringUtils::format("Town_Hall%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }

    
    return true;
}