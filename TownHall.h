#ifndef __TOWNHALL_H__
#define __TOWNHALL_H__

#include "Building.h"
#include"BuildingManager.h"
#include"vector"

class TownHall : public Building
{
public:
    // 创建方法
    static TownHall* create(const std::string& buildingFile = "Town_Hall/Town_Hall1.png", 
        bool isHomeTown = true,
        const std::string& turfFile = "grass.png",
        float buildingScale = 0.75f);

    TownHall() = default;
    ~TownHall() = default;

protected:
    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale) override;

    void upgrade() override;
    int _maxCapacity;       //资源最大存储量
	std::vector<int> capacityList;
	
};

#endif // __TOWNHALL_H__