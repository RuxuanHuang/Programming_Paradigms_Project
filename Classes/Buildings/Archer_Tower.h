#ifndef __ARCHERTOWER_H__
#define __ARCHERTOWER_H__

#include "Building.h"

class ArcherTower : public Building
{
public:
    // 创建方法
    static ArcherTower* create(const std::string& buildingFile = "Ancher_Tower/Archer_Tower1.png",
        bool isHownTown = true,
        const std::string& turfFile = "grass.png",
        float buildingScale = 0.8f);

    ArcherTower() = default;
    ~ArcherTower() = default;
    virtual void upgrade() override;


protected:
    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale) override;
private:
    int _attackingPerHour;
    std::vector<int>attackingPerHourList;

};

#endif 
