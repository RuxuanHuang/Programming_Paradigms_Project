#ifndef __ARCHERTOWER_H__
#define __ARCHERTOWER_H__

#include "Building.h"
class Soldier;
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
    void ArcherTower::scanForTargets(float dt);
    void ArcherTower::startCombatAI();
protected:
    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale) override;
private:
    int _attackingPerSecond;
    float _attackingRange;
    std::vector<int>attackingPerSecondList;
private:
    Soldier* _currentTarget = nullptr; // 记录当前锁定的目标

};

#endif 
