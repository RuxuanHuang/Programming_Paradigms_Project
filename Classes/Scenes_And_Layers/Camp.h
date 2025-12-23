

#ifndef __CAMP_H__
#define __CAMP_H__

#include "cocos2d.h"
#include "Building.h" // 包含基类头文件
#include "TownHall.h" // 包含子类头文件
#include "BuildersHut.h"
#include"Store.h"
#include "BattleScene1.h" 

class Camp : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    void menuCloseCallback(cocos2d::Ref* pSender);
    CREATE_FUNC(Camp);

    void selectBuilding(Building* building);
    void clearSelection();
    void changeMapSkin();
    void enterBattleScene();
private:
    // --- 场景内部变量 ---

    // 使用基类指针存储建筑对象
    Building* _building;

    // 地图拖动状态
    bool _isMapDragging;
    cocos2d::Vec2 _lastMapMousePos;

    // --- 地图/场景事件回调 ---
    void onMapMouseDown(cocos2d::Event* event);
    void onMapMouseUp(cocos2d::Event* event);
    void onMapMouseMove(cocos2d::Event* event);
    void onMouseScroll(cocos2d::Event* event);

    // 边界限制
    void limitMapPos(cocos2d::Sprite* sprite);

    Building* _selectedBuilding = nullptr;
    bool _mapMoved = false;

    Store* _store = nullptr;
    // 新增：保存操作栏实例（用于设置换肤回调）
    BuildingActionBar* _globalActionBar = nullptr;
    // 新增：当前地图皮肤名称
    std::string _currentMapSkin = "others/Camp.png";

};

#endif // __CAMP_H__