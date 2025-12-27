#include "BuildingActionBar.h"
#include "Building.h"

USING_NS_CC;
const int LABEL_FONT_SIZE = 24;


// 构造函数
BuildingActionBar::BuildingActionBar() :
    _currentBuilding(nullptr),
    _infoButton(nullptr),
    _upgradeButton(nullptr),
    _changeSkinButton(nullptr),
    _buttonContainer(nullptr),
    _costLabel(nullptr),
    _resourceIcon(nullptr), 
    _infoCallback(nullptr),
    _upgradeCallback(nullptr),
    _changeSkinCallback(nullptr)

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
    this->setPosition(Vec2(visibleSize.width / 2, 0)); 
    // 创建按钮
    createButtons();
    //创建金币/圣水图标
    createResourceIcon();

    // 初始状态隐藏
    this->setVisible(false);
    this->setOpacity(0);

    return true;
}

// 创建按钮
void BuildingActionBar::createButtons()
{
    _buttonContainer = Node::create();
    this->addChild(_buttonContainer);

    // 1. 信息按钮
    _infoButton = Button::create("Buttons/InformationButton.png", "", "");
    if (_infoButton) {
        // 移除所有额外设置，只保留默认图片
        _infoButton->setScale(0.24f);
        _infoButton->setPosition(Vec2(-400, -200));
        _buttonContainer->addChild(_infoButton);
    }

    // 2. 切换外观按钮
    _changeSkinButton = Button::create("Buttons/ChangeAppearance.png", "", ""); 
    if (_changeSkinButton) {
        _changeSkinButton->setScale(0.21f);
        _changeSkinButton->setPosition(Vec2(0, -190)); // 中间位置
        _changeSkinButton->setVisible(false); // 默认隐藏
        _buttonContainer->addChild(_changeSkinButton);
    }

    // 3. 升级按钮
    _upgradeButton = Button::create("Buttons/UpgradeButton.png", "", "");
    if (_upgradeButton) {
        _upgradeButton->setScale(0.21f);
        _upgradeButton->setPosition(Vec2(400, -200)); 
        _buttonContainer->addChild(_upgradeButton);
    }
    
    // 创建标签 - 调整到按钮外部
    _costLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 22); 
    if (!_costLabel) {
        _costLabel = Label::createWithSystemFont("", "Arial", 22);
    }

    if (_costLabel) {
        _buttonContainer->addChild(_costLabel, 100);

        // 调整位置，确保在按钮上方
        _costLabel->setPosition(Vec2(400, -160)); // 在升级按钮上方
        _costLabel->setColor(Color3B::YELLOW);
        _costLabel->enableOutline(Color4B::BLACK, 2);

       
    }

    // 设置按钮回调
    setupButtonCallbacks();
}


// 创建资源图标
void BuildingActionBar::createResourceIcon()
{
    // 创建资源图标（初始隐藏）
    _resourceIcon = Sprite::create();
    _resourceIcon->setVisible(false);
    _resourceIcon->setScale(0.6f);

    if (_buttonContainer) {
        _buttonContainer->addChild(_resourceIcon, 101); // 较高层级
    }
}

// 根据资源类型获取图标路径
std::string BuildingActionBar::getResourceIconPath(const std::string& resourceType) const
{
    
    if (resourceType == "Gold") {
        return "others/GoldIcon.png";  // 金币图标
    }
    else if (resourceType == "Elixir") {
        return "others/ElixirIcon.png"; // 圣水图标
    }
    else {
       
        CCLOG("Warning: Unknown resource type: %s", resourceType.c_str());
        return "others/GoldIcon.png";
    }
}

// 更新资源图标显示
void BuildingActionBar::updateResourceIcon(int cost, const std::string& resourceType)
{
    if (!_resourceIcon) return;

    if (cost > 0 && !resourceType.empty()) {
        // 显示资源图标
        std::string iconPath = getResourceIconPath(resourceType);
        _resourceIcon->setTexture(iconPath);

        if (_resourceIcon->getTexture()) {
            _resourceIcon->setVisible(true);

            
            if (_costLabel && _upgradeButton) {
                // 获取价格标签的宽度
                float labelWidth = _costLabel->getContentSize().width;

                // 设置资源图标位置
                float iconX = _costLabel->getPositionX() + labelWidth / 2 + 15;
                float iconY = _costLabel->getPositionY()+5 ;

                _resourceIcon->setPosition(Vec2(iconX, iconY));
            }
        }
        else {
            // 如果加载图标失败，隐藏图标
            CCLOG("Error: Failed to load resource icon: %s", iconPath.c_str());
            _resourceIcon->setVisible(false);
        }
    }
    else {
        // 如果没有成本或者资源类型为空，隐藏图标
        _resourceIcon->setVisible(false);
    }
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
    if (_changeSkinButton) {
        _changeSkinButton->addClickEventListener([this](Ref* sender) {
            if (_changeSkinCallback) _changeSkinCallback();
            });
    }
}

// 显示操作栏
void BuildingActionBar::showForBuilding(Building* building)
{
    if (!building) return;

    _currentBuilding = building;

	int cost = building->getCost();
    std::string resourceType = building->getCostType();
    if (cost) {
        _costLabel->setString(std::to_string(cost));
        updateResourceIcon(cost, resourceType);
    }
    else {
        _costLabel->setString("");
    }
    
    if (_changeSkinButton) {
        _changeSkinButton->setVisible(false);
    }

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

void BuildingActionBar::showForTownHall(Building* building)
{
    if (!building) return;

    // 先调用普通显示逻辑
    showForBuilding(building);

    // 显示切换外观按钮
    if (_changeSkinButton) {
        _changeSkinButton->setVisible(true);
    }

    CCLOG("操作栏显示: %s (大本营，显示切换外观按钮)", building->getBuildingName().c_str());
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

void BuildingActionBar::setChangeSkinCallback(const std::function<void()>&callback)
{
    _changeSkinCallback = callback;
}

// 回调设置
void BuildingActionBar::setCallbacks(const std::function<void()>& infoCallback,
    const std::function<void()>& upgradeCallback)
{
    _infoCallback = infoCallback;
    _upgradeCallback = upgradeCallback;
}