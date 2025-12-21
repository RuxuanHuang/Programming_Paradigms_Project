#include "BuildersHut.h"

USING_NS_CC;

BuildersHut* BuildersHut::create(const std::string& buildingFile,
    bool isHownTown,
    const std::string& turfFile,
    float buildingScale)
{
    BuildersHut* ret = new (std::nothrow) BuildersHut();
    ret->setIsHownTown(isHownTown);
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}




bool BuildersHut::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    // 1. 先调用父类的init
    if (!Building::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 2. 设置大本营基本属性
    this->setBuildingName("Builders Hut");
    this->setBuildingSize(2);
    this->setBuildingScale(0.9f);
    this->setMaxLevel(1);

   

    return true;
}

