#ifndef __CANNON_H__
#define __CANNON_H__

#include "Building.h"

class Cannon : public Building
{
public:
    // 创建方法
    static Cannon* create(const std::string& buildingFile = "Cannon/Cannon1.png",
        bool isHownTown = true,
        const std::string& turfFile = "grass.png",
        float buildingScale = 0.8f);

    Cannon() = default;
    ~Cannon() = default;
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