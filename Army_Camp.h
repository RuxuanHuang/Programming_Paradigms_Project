#ifndef __ARMYCAMP_H__
#define __ARMYCAMP_H__

#include "Building.h"
#include"people.h"
class ArmyCamp : public Building
{
public:
    // 创建方法
    static ArmyCamp* create(const std::string& buildingFile = "Army_Camp/Army_Camp1.png",
        bool isHomeTown = true,
        const std::string& turfFile = "grass.png",
        float buildingScale = 0.8f);

    ArmyCamp() = default;
    ~ArmyCamp() = default;

    virtual void upgrade() override;
	int getPopulationCapacity() const { return _populationCapcity; }
protected:
    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale) override;
private:
    int _populationCapcity;
    std::vector<int>populationCapacityList;
};

#endif 