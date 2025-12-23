#include "Army_Camp.h"
USING_NS_CC;

ArmyCamp* ArmyCamp::create(const std::string& buildingFile,
    bool isHownTown,
    const std::string& turfFile,
    float buildingScale)
{
    ArmyCamp* ret = new (std::nothrow) ArmyCamp();
    ret->setIsHownTown(isHownTown);
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}


bool ArmyCamp::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    // 1. 先调用父类的init
    if (!Building::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 2. 设置基本属性
    this->setBuildingName("Army Camp");
    this->setBuildingSize(4);
    this->setCost(2000);
    this->setHP(100);

    int hpValues[] = { 100,150,200 };
    int upgradeCosts[] = { 2000,15000,0 };
    // 3. 设置等级信息
    for (int i = 1; i <= 3; i++) {
        _upgradeSprites[i] = levelInformation{ hpValues[i - 1], upgradeCosts[i - 1], "" };
        std::string spriteFile = StringUtils::format("Army_Camp/Army_Camp%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }

    _populationCapcity = 20;
    populationCapacityList = { 20,30,35 };
    return true;
}

void ArmyCamp::upgrade()
{
    // 先调用基类的升级逻辑
    Building::upgrade();

    _populationCapcity = populationCapacityList[_level - 1];
}