#ifndef __BUILDERSHUT_H__
#define __BUILDERSHUT_H__

#include "Building.h"
//原版中工人小屋需要14级大本营才可以升级，此处先把升级功能限制掉
class BuildersHut : public Building
{
public:
    // 创建方法
    static BuildersHut* create(const std::string& buildingFile = "Builders_Hut1.png",
        bool isHownTown = true,
        const std::string& turfFile = "grass.png",
        float buildingScale = 0.8f);

    BuildersHut() = default;
    ~BuildersHut() = default;

    

protected:
    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale) override;
};

#endif 