#ifndef __TOWNHALL_H__
#define __TOWNHALL_H__

#include "Building.h"

class TownHall : public Building
{
public:
    // 创建方法
    static TownHall* create(const std::string& buildingFile = "townhall1.png",
        const std::string& turfFile = "grass.png",
        float buildingScale = 0.7f);

    TownHall() = default;
    ~TownHall() = default;

protected:
    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale) override;
};

#endif // __TOWNHALL_H__