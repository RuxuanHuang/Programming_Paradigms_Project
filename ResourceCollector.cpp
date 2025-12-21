#include "ResourceCollector.h"

bool ResourceCollector::init(const std::string& buildingFile, const std::string turfFile, float buildingScale) {
    if (!Building::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 初始化默认属性
    _currentStored = 0;
    _productionPerHour = 2000;
    _maxCapacity = 1000;
    _collectThreshold = 0.03f; // 达到10%容量显示图标

    this->setBuildingScale(0.85);
    this->setCost(300);
    this->setHP(75);
    this->setMaxLevel(6);
    storageCapacityList = { 1000,2000,3000,5000,10000,20000 };
    int hpValues[] = { 75,150,300,400,500,550 };
    int upgradeCosts[] = { 300,700,1400,3000,7000,0 };
    for (int i = 1; i <= 6; i++) {
        _upgradeSprites[i] = levelInformation{ hpValues[i - 1], upgradeCosts[i - 1], "" };

    }
    // 启动生产计时器
    this->schedule(CC_SCHEDULE_SELECTOR(ResourceCollector::updateProduction), 1.0f); // 每秒计算一次

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
        _collectIcon->setPosition(cocos2d::Vec2(0, iconY + 30));
        this->addChild(_collectIcon, 20);
        _collectIcon->setVisible(false);

        // 给图标加一个简单的浮动动画
        auto moveUp = cocos2d::MoveBy::create(0.8f, cocos2d::Vec2(0, 10));
        auto moveDown = moveUp->reverse();
        _collectIcon->runAction(cocos2d::RepeatForever::create(cocos2d::Sequence::create(moveUp, moveDown, nullptr)));
    }
}

void ResourceCollector::updateIconVisibility() {
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

    _maxCapacity =storageCapacityList[_level - 1];
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

GoldMine* GoldMine::create(const std::string& buildingFile, const std::string turfFile, float buildingScale)
{
    GoldMine* ret = new (std::nothrow) GoldMine();
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
   

   
    setupCollectIcon("GoldIcon.png");
    for (int i = 1; i <= 6; i++) {

        std::string spriteFile = StringUtils::format("Gold_Mine%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }


    return true;
}

void GoldMine::onCollected(float amount)
{
    // 这里处理金币逻辑
    int goldToAdd = static_cast<int>(amount);
    CCLOG("Collected %d Gold from Mine!", goldToAdd);

    // 示例：调用你的全局单例或玩家数据类增加金币
    // PlayerData::getInstance()->addGold(goldToAdd);

    // 可以在这里添加一个飘字动画或者金币飞向UI栏的特效
}
ElixirCollector* ElixirCollector::create(const std::string& buildingFile, const std::string turfFile, float buildingScale)
{
    ElixirCollector* ret = new (std::nothrow) ElixirCollector();
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
    setupCollectIcon("ElixirIcon.png");
    for (int i = 1; i <= 6; i++) {

        std::string spriteFile = StringUtils::format("Elixir_Collector%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }


    return true;
}

void ElixirCollector::onCollected(float amount)
{
    
    int goldToAdd = static_cast<int>(amount);
   

    // 示例：调用你的全局单例或玩家数据类增加金币
    // PlayerData::getInstance()->addGold(goldToAdd);

    // 可以在这里添加一个飘字动画或者金币飞向UI栏的特效
}