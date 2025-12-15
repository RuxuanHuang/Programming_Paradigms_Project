// Classes/Building/BuildingActionBar.h
#ifndef __BUILDING_ACTION_BAR_H__
#define __BUILDING_ACTION_BAR_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace ui;


class Building;

// 最简单的操作栏
class BuildingActionBar : public Node
{
public:
    static BuildingActionBar* getInstance();
    static void destroyInstance();

    void showForBuilding(Building* building);
    void hide();

    void setCallbacks(const std::function<void()>& infoCallback,
        const std::function<void()>& upgradeCallback);

private:
    BuildingActionBar();
    virtual ~BuildingActionBar() = default;
    bool init();

    void createButtons();
    void setupButtonCallbacks();

private:
    static BuildingActionBar* _instance;
    Building* _currentBuilding;

    Button* _infoButton;     // 只显示Information.png
    Button* _upgradeButton;  // 只显示Upgrade.png
    Node* _buttonContainer;

    std::function<void()> _infoCallback;
    std::function<void()> _upgradeCallback;
};

#endif // __BUILDING_ACTION_BAR_H__