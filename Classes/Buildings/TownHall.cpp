#include "TownHall.h"
#include"resources.h"

USING_NS_CC;

TownHall* TownHall::create(const std::string& buildingFile,
    bool isHomeTown,
    const std::string& turfFile,
    float buildingScale)
{
    TownHall* ret = new (std::nothrow) TownHall();
    ret->setIsHownTown(isHomeTown);
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
    if (!Building::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 设置大本营基本属性
    this->setBuildingName("Town Hall");
    this->setBuildingSize(4);
    this->setCost(1000);
    this->setHP(400);

    _maxCapacity = 2500;
    capacityList = { 2500,10000,50000 };
    

    int hpValues[] = { 400,800,1600 };
    int upgradeCosts[] = { 1000,4000,0 };
    // 3. 设置大本营等级信息
    for (int i = 1; i <= 3; i++) {
        _upgradeSprites[i] = levelInformation{ hpValues[i - 1], upgradeCosts[i - 1], "" };
        std::string spriteFile = StringUtils::format("Town_Hall/Town_Hall%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }
    return true;

}

void TownHall::upgrade()
{
    Building::upgrade();

	_maxCapacity = capacityList[_level - 1];
    //建筑管理系统里更新大本营的升级
    BuildingManager* buildingManager = BuildingManager::getInstance();
    buildingManager->addTownhallLevel();
    //资源管理系统更新金币圣水容量的相应增加
    ResourceManager* resourceManager = ResourceManager::getInstance();
    resourceManager->updateElixirMaxLimit(_maxCapacity- capacityList[_level-2]);
    resourceManager->updateGoldMaxLimit(_maxCapacity - capacityList[_level - 2]);

}

std::vector<std::pair<std::string, std::string>> TownHall::getSpecificInfoItems()
{
    return {
        {"Gold Capacity", std::to_string(static_cast<int>(_maxCapacity))},
        {"Elixir Capacity", std::to_string(static_cast<int>(_maxCapacity))}
    };
}