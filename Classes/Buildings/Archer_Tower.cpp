#include "Archer_Tower.h"

USING_NS_CC;

ArcherTower* ArcherTower::create(const std::string& buildingFile,
    bool isHownTown,
    const std::string& turfFile,
    float buildingScale)
{
    ArcherTower* ret = new (std::nothrow) ArcherTower();
    ret->setIsHownTown(isHownTown);
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ArcherTower::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    // 1. 先调用父类的init
    if (!Building::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 2. 设置基本属性
    this->setBuildingName("Archer_Tower");
    this->setBuildingSize(3);
    this->setBuildingScale(0.9f);
    this->setMaxLevel(3);
    this->setCost(1000);



    int hpValues[] = { 380,420,460 };
    int upgradeCosts[] = { 2000,5000,0 };
    // 3. 设置等级信息
    for (int i = 1; i <= 3; i++) {
        _upgradeSprites[i] = levelInformation{ hpValues[i - 1], upgradeCosts[i - 1], "" };
        std::string spriteFile = StringUtils::format("Archer_Tower/Archer_Tower%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }

    _attackingPerHour = 11;
    attackingPerHourList = { 11,15,19 };
    return true;
}

void ArcherTower::upgrade()
{
    // 先调用基类的升级逻辑
    Building::upgrade();

    _attackingPerHour = attackingPerHourList[_level - 1];
}


