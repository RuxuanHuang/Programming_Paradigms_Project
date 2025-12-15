
#include "BuildingActionBar.h"
#include "Building.h"

USING_NS_CC;

// 静态变量初始化
BuildingActionBar* BuildingActionBar::_instance = nullptr;

// 获取单例
BuildingActionBar* BuildingActionBar::getInstance()
{
    if (!_instance) {
        _instance = new (std::nothrow) BuildingActionBar();
        if (_instance && _instance->init()) {
            _instance->autorelease();
        }
        else {
            CC_SAFE_DELETE(_instance);
            _instance = nullptr;
        }
    }
    return _instance;
}

// 销毁单例
void BuildingActionBar::destroyInstance()
{
    if (_instance) {
        _instance->removeFromParent();
        _instance = nullptr;
    }
}

// 构造函数
BuildingActionBar::BuildingActionBar() :
    _currentBuilding(nullptr),
    _infoButton(nullptr),
    _upgradeButton(nullptr),
    _buttonContainer(nullptr)
{
}

// 初始化
bool BuildingActionBar::init()
{
    if (!Node::init()) {
        return false;
    }

    // 设置屏幕底部居中位置
    auto visibleSize = Director::getInstance()->getVisibleSize();
    this->setPosition(Vec2(visibleSize.width / 2, 300)); // 离底部300像素

    // 创建最简单的按钮
    createButtons();

    // 初始状态：隐藏
    this->setVisible(false);
    this->setOpacity(0);

    return true;
}

// 创建按钮 - 最简单版本
void BuildingActionBar::createButtons()
{
    _buttonContainer = Node::create();
    this->addChild(_buttonContainer);

    // 1. 信息按钮
    _infoButton = Button::create("Information.png", "", "");
    if (_infoButton) {
        // 移除所有额外设置，只保留默认图片
        _infoButton->setScale(0.12f);
        _infoButton->setPosition(Vec2(200, 0));
        _buttonContainer->addChild(_infoButton);
    }

    // 2. 升级按钮
    _upgradeButton = Button::create("Upgrade.png", "", "");
    if (_upgradeButton) {
        _upgradeButton->setScale(0.12f);
        _upgradeButton->setPosition(Vec2(-200, 0)); 
        _buttonContainer->addChild(_upgradeButton);
    }

    // 设置按钮回调
    setupButtonCallbacks();
}

// 设置按钮回调
void BuildingActionBar::setupButtonCallbacks()
{
    if (_infoButton) {
        _infoButton->addClickEventListener([this](Ref* sender) {
            if (_infoCallback) {
                _infoCallback();
            }
            });
    }

    if (_upgradeButton) {
        _upgradeButton->addClickEventListener([this](Ref* sender) {
            if (_upgradeCallback) {
                _upgradeCallback();
            }
            });
    }
}

// 显示操作栏
void BuildingActionBar::showForBuilding(Building* building)
{
    if (!building) return;

    _currentBuilding = building;

    // 显示动画
    if (!this->isVisible()) {
        this->setVisible(true);
        this->setPositionY(-100); // 从屏幕外开始

        auto moveUp = MoveTo::create(0.3f, Vec2(this->getPositionX(), 300));
        auto fadeIn = FadeIn::create(0.3f);
        this->runAction(Spawn::create(moveUp, fadeIn, nullptr));
    }

    CCLOG("操作栏显示: %s", building->getBuildingName().c_str());
}

// 隐藏操作栏
void BuildingActionBar::hide()
{
    if (this->isVisible()) {
        auto moveDown = MoveTo::create(0.2f, Vec2(this->getPositionX(), -100));
        auto fadeOut = FadeOut::create(0.2f);
        auto hideAction = CallFunc::create([this]() {
            this->setVisible(false);
            _currentBuilding = nullptr;
            });

        this->runAction(Sequence::create(
            Spawn::create(moveDown, fadeOut, nullptr),
            hideAction,
            nullptr
        ));

        CCLOG("操作栏隐藏");
    }
}

// 设置回调函数
void BuildingActionBar::setCallbacks(const std::function<void()>& infoCallback,
    const std::function<void()>& upgradeCallback)
{
    _infoCallback = infoCallback;
    _upgradeCallback = upgradeCallback;
}