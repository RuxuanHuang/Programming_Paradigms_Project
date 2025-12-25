#ifndef __WALL_H__
#define __WALL_H__

#include "Building.h"

class Wall : public Building
{
public:
    // 创建方法
    static Wall* create(const std::string& buildingFile = "Wall/Wall1.png",
        bool isHomeTown = true,
        const std::string& turfFile = "grass.png",
        float buildingScale = 0.8f);

    Wall() = default;
    ~Wall() = default;


protected:
    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale) override;


};

#endif 
