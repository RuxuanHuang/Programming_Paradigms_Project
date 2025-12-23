#include "ResourceStorageBuilding.h"

USING_NS_CC;

ResourceStorageBuilding::ResourceStorageBuilding() :
    _maxStorage(1000),
    _currentStorage(0)
{
}

ResourceStorageBuilding* ResourceStorageBuilding::create(const std::string& buildingFile,
    bool isHownTown,
    const std::string turfFile,
    float buildingScale)
{
    ResourceStorageBuilding* ret = new (std::nothrow) ResourceStorageBuilding();
    ret->setIsHownTown(isHownTown);
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ResourceStorageBuilding::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    if (!Building::init(buildingFile, turfFile, buildingScale))
    {
        return false;
    }
    this->setBuildingScale(0.85);
    this->setCost(750);
    this->setHP(150);
    this->setMaxLevel(6);
    storageCapacityList = { 1500,3000,6000,12000,25000,45000 };
    int hpValues[] = { 150,300,450,600,800,1000 };
    int upgradeCosts[] = { 750,1500,3000,6000,12000,0 };

    for (int i = 1; i <= 6; i++) {
        _upgradeSprites[i] = levelInformation{ hpValues[i - 1], upgradeCosts[i - 1], "" };

    }

    return true;
}




void ResourceStorageBuilding::upgrade()
{
    // 先调用基类的升级逻辑
    Building::upgrade();

    setMaxStorage(storageCapacityList[_level - 1]);
}


GoldStorage* GoldStorage::create(const std::string& buildingFile,
    bool isHownTown,
    const std::string turfFile,
    float buildingScale)
{
    GoldStorage* ret = new (std::nothrow) GoldStorage();
    ret->setIsHownTown(isHownTown);
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}



bool GoldStorage::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    // 1. 先调用父类的init
    if (!ResourceStorageBuilding::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 2. 设置基本属性
    this->setBuildingName("Gold Storage");



    // 3. 设置等级信息
    for (int i = 1; i <= 6; i++) {

        std::string spriteFile = StringUtils::format("Gold_Storage/Gold_Storage%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }

    return true;
}


ElixirStorage* ElixirStorage::create(const std::string& buildingFile,
    bool isHownTown,
    const std::string turfFile,
    float buildingScale)
{
    ElixirStorage* ret = new (std::nothrow) ElixirStorage();
    ret->setIsHownTown(isHownTown);
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}


bool ElixirStorage::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    // 1. 先调用父类的init
    if (!ResourceStorageBuilding::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 2. 设置基本属性
    this->setBuildingName("Elixir Storage");
    this->setBuildingScale(0.85);


    // 3. 设置等级信息
    for (int i = 1; i <= 6; i++) {

        std::string spriteFile = StringUtils::format("Elixir_Storage/Elixir_Storage%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }

    return true;
}