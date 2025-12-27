#include "ResourceCollector.h"
#include"resources.h"

bool ResourceCollector::init(const std::string& buildingFile, const std::string turfFile, float buildingScale) {
    if (!Building::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 初始化默认属性

    _currentStored = 0;
    _productionPerHour = 2000;
    _maxCapacity = 1000;
    _collectThreshold = 0.03f; // 达到3%容量显示图标


    this->setBuildingScale(0.85);
    this->setBuildingSize(3);
    this->setCost(300);
    this->setHP(75);
    this->setMaxLevel(6);
    storageCapacityList = { 1000,2000,3000,5000,10000,20000 };
    int hpValues[] = { 75,150,300,400,500,550 };
    int upgradeCosts[] = { 300,700,1400,3000,7000,0 };
    for (int i = 1; i <= 6; i++) {
        _upgradeSprites[i] = levelInformation{ hpValues[i - 1], upgradeCosts[i - 1], "" };

    }
    if (_isHomeTown==true) {
        // 启动生产计时器
        this->schedule(CC_SCHEDULE_SELECTOR(ResourceCollector::updateProduction), 1.0f); // 每秒计算一次
    }
    else {
        _currentStored = _maxCapacity;
    }
    

    return true;
}

void ResourceCollector::updateProduction(float dt) {
    if (_currentStored >= _maxCapacity) return;

    // 计算这一秒产出的资源量： (每小时产量 / 3600秒) * 经过的秒数
    float produced = (_productionPerHour / 3600.0f) * dt;
    _currentStored = std::min(_maxCapacity, _currentStored + produced);

    updateIconVisibility();
}

void ResourceCollector::setupCollectIcon(const std::string& iconFile) {
    _collectIcon = cocos2d::Sprite::create(iconFile);
    if (_collectIcon) {
        // 将图标放在建筑上方
        float iconY = _buildingSprite->getContentSize().height * _buildingSprite->getScaleY();
        _collectIcon->setPosition(cocos2d::Vec2(0, iconY + 10));
        this->addChild(_collectIcon, 10);
        _collectIcon->setVisible(false);

        // 给图标加一个简单的浮动动画
        auto moveUp = cocos2d::MoveBy::create(0.8f, cocos2d::Vec2(0, 10));
        auto moveDown = moveUp->reverse();
        _collectIcon->runAction(cocos2d::RepeatForever::create(cocos2d::Sequence::create(moveUp, moveDown, nullptr)));
    }
}

void ResourceCollector::updateIconVisibility() {
	if (!_isHomeTown) return;
    if (!_collectIcon) return;

    // 判断是否超过阈值
    bool shouldShow = (_currentStored / _maxCapacity) >= _collectThreshold;

    if (shouldShow && !_collectIcon->isVisible()) {
        _collectIcon->setVisible(true);
        _collectIcon->setScale(0);
        _collectIcon->runAction(cocos2d::EaseBackOut::create(cocos2d::ScaleTo::create(0.3f, 1.0f)));
    }
    else if (!shouldShow) {
        _collectIcon->setVisible(false);
    }
}


void ResourceCollector::upgrade()
{
    // 先调用基类的升级逻辑
    Building::upgrade();

    _maxCapacity = storageCapacityList[_level - 1];
}


void ResourceCollector::collectResource() {
    if (_currentStored > 0) {
        // 调用子类的具体加算逻辑
        onCollected(_currentStored);

        // 重置存储量
        _currentStored = 0;
        updateIconVisibility();


    }
}

void ResourceCollector::onBuildingMouseDown(Event* event) {
    auto e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT)
        return;

    // 首先检查是否在建筑范围内（基类的方法）
    if (!isClickingInTurf(this->_parent, e))
        return;

    // 如果可以收集，记录状态
    if (isCollectable()) {
        _mouseStartedOnCollectable = true;
        _shouldCollect = true;
        _mouseStartPos = cocos2d::Vec2(e->getCursorX(), e->getCursorY());

        // 阻止事件传播，这样就不会触发建筑选中和拖动
        e->stopPropagation();
        return;
    }

    // 如果不能收集，调用基类方法（正常建筑逻辑）
    Building::onBuildingMouseDown(event);
}

void ResourceCollector::onBuildingMouseUp(Event* event) {
    auto e = static_cast<EventMouse*>(event);

    if (_mouseStartedOnCollectable && _shouldCollect) {
        // 检查是否是有效的点击（不是拖动）
        cocos2d::Vec2 mouseEndPos = cocos2d::Vec2(e->getCursorX(), e->getCursorY());
        float distance = _mouseStartPos.distance(mouseEndPos);

        if (distance <= _clickThreshold) {
            // 执行收集操作
            tryCollectResource();
        }

        // 恢复图标大小
        if (_collectIcon) {
            _collectIcon->runAction(cocos2d::Sequence::create(
                cocos2d::ScaleTo::create(0.1f, 1.0f),
                nullptr
            ));
        }

        // 重置状态
        _mouseStartedOnCollectable = false;
        _shouldCollect = false;

        // 阻止事件传播，防止触发建筑选中
        e->stopPropagation();
        return;
    }

    // 如果不是收集操作，调用基类方法
    Building::onBuildingMouseUp(event);

    // 重置状态
    _mouseStartedOnCollectable = false;
    _shouldCollect = false;
}

void ResourceCollector::onBuildingMouseMove(Event* event) {
    auto e = static_cast<EventMouse*>(event);

    if (_mouseStartedOnCollectable && _shouldCollect) {
        // 检查移动距离
        cocos2d::Vec2 currentMousePos = cocos2d::Vec2(e->getCursorX(), e->getCursorY());
        float distance = _mouseStartPos.distance(currentMousePos);

        if (distance > _clickThreshold) {
            // 移动距离超过阈值，取消收集状态
            _shouldCollect = false;

            // 恢复图标大小
            if (_collectIcon) {
                _collectIcon->runAction(cocos2d::Sequence::create(
                    cocos2d::ScaleTo::create(0.1f, 1.0f),
                    nullptr
                ));
            }

            // 现在可以拖动地图或建筑了
            // 调用基类的onBuildingMouseMove来处理拖动
            Building::onBuildingMouseMove(event);
            return;
        }

        // 移动距离小，保持收集状态，阻止其他处理
        e->stopPropagation();
        return;
    }

    // 如果不是收集操作，调用基类方法
    Building::onBuildingMouseMove(event);
}

void ResourceCollector::tryCollectResource() {
    if (isCollectable()) {
        // 播放收集音效
        cocos2d::AudioEngine::play2d("collect_sound.mp3", false, 0.8f);

        // 执行收集
        collectResource();
    }
}

std::vector<std::pair<std::string, std::string>> ResourceCollector::getSpecificInfoItems()
{
    return {
        {"Production", std::to_string(static_cast<int>(_productionPerHour)) + "/h"},
        {"Storage", std::to_string(static_cast<int>(_currentStored)) + "/" + std::to_string(static_cast<int>(_maxCapacity))}
    };
}

GoldMine* GoldMine::create(const std::string& buildingFile, bool isHomeTown, const std::string turfFile, float buildingScale)
{

    GoldMine* ret = new (std::nothrow) GoldMine();
    ret->_isHomeTown = isHomeTown;
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {

        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool GoldMine::init(const std::string& buildingFile, const std::string turfFile, float buildingScale)
{
    // 1. 调用基类 init
    if (!ResourceCollector::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 2. 设置金矿特有的属性
    _buildingName = "Gold Mine";
    _costType = "Elixir";


    setupCollectIcon("others/GoldIcon.png");
    for (int i = 1; i <= 6; i++) {

        std::string spriteFile = StringUtils::format("Gold_Mine/Gold_Mine%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }


    return true;
}

void GoldMine::onCollected(float amount) {
    int goldToAdd = static_cast<int>(amount);
    CCLOG("Collected %d Gold from Mine!", goldToAdd);

    // 调用资源管理器
    auto resourceManager = ResourceManager::getInstance();
    resourceManager->earnGold(goldToAdd);

    
}


ElixirCollector* ElixirCollector::create(const std::string& buildingFile, bool isHomeTown, const std::string turfFile, float buildingScale)
{
    ElixirCollector* ret = new (std::nothrow) ElixirCollector();
    ret->_isHomeTown = isHomeTown;
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {

        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ElixirCollector::init(const std::string& buildingFile, const std::string turfFile, float buildingScale)
{

    if (!ResourceCollector::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }
    _buildingName = "Gold Mine";
    setupCollectIcon("others/ElixirIcon.png");
    for (int i = 1; i <= 6; i++) {

        std::string spriteFile = StringUtils::format("Elixir_Collector/Elixir_Collector%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }


    return true;
}

void ElixirCollector::onCollected(float amount) {
    int elixirToAdd = static_cast<int>(amount);
    CCLOG("Collected %d Elixir from Collector!", elixirToAdd);

    // 调用资源管理器
    auto resourceManager = ResourceManager::getInstance();
    resourceManager->earnElixir(elixirToAdd);

    
}