#include "Cannon.h"

USING_NS_CC;

Cannon* Cannon::create(const std::string& buildingFile,
    bool isHownTown,
    const std::string& turfFile,
    float buildingScale)
{
    Cannon* ret = new (std::nothrow) Cannon();
    ret->setIsHownTown(isHownTown);
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Cannon::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    // 1. 先调用父类的init
    if (!Building::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 2. 设置大本营基本属性
    this->setBuildingName("Cannon");
    this->setBuildingSize(3);
    this->setBuildingScale(0.9f);
    this->setMaxLevel(4);
    this->setCost(1000);



    int hpValues[] = { 300,360,420,500 };
    int upgradeCosts[] = { 1000,4000,16000,0 };
    // 3. 设置等级信息
    for (int i = 1; i <= 4; i++) {
        _upgradeSprites[i] = levelInformation{ hpValues[i - 1], upgradeCosts[i - 1], "" };
        std::string spriteFile = StringUtils::format("Cannon/Cannon%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }

    _attackingPerHour = 7;
    attackingPerHourList = { 7,10,13,17 };
    return true;
}

void Cannon::upgrade()
{
    // 先调用基类的升级逻辑
    Building::upgrade();

    _attackingPerHour = attackingPerHourList[_level - 1];
}

