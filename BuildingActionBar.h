
#ifndef __BUILDING_ACTION_BAR_H__
#define __BUILDING_ACTION_BAR_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace ui;


class Building;

class BuildingActionBar : public Node
{
public:
    //static BuildingActionBar* getInstance();
    //static void destroyInstance();
    CREATE_FUNC(BuildingActionBar);
    void showForBuilding(Building* building);
    void showForTownHall(Building* building);
    void hide();

    // 新增：设置切换外观按钮的回调（由Camp设置）
    void setChangeSkinCallback(const std::function<void()>& callback);
    // 原有回调设置
    void setCallbacks(const std::function<void()>& infoCallback,
        const std::function<void()>& upgradeCallback);


private:
    BuildingActionBar();
    virtual ~BuildingActionBar() = default;
    bool init();

    void createButtons();
    void setupButtonCallbacks();


    //static BuildingActionBar* _instance;
    Building* _currentBuilding;

    Button* _infoButton;     // 只显示Information.png
    Button* _upgradeButton;  // 只显示Upgrade.png
    Button* _changeSkinButton;  //外观
    Node* _buttonContainer;
    Label* _costLabel;      // 信息标签
    std::function<void()> _infoCallback;
    std::function<void()> _upgradeCallback;
    std::function<void()> _changeSkinCallback;
};

#endif // __BUILDING_ACTION_BAR_H__