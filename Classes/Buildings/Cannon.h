#ifndef __CANNON_H__
#define __CANNON_H__

#include "Building.h"
class Soldier;
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


    void startCombatAI(); // 新增：启动战斗AI
protected:
    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale) override;
    void scanForTargets(float dt); // 每秒执行的扫描函数
private:
    int _attackingPerSecond;
    float _attackingRange;
    std::vector<int>attackingPerSecondList;
private:
    Soldier* _currentTarget = nullptr; // 记录当前锁定的目标

};

#endif 