#include "Building.h"
#include"BuildingActionBar.h"
#include"Camp.h"
#include"BattleScene1.h"
#include "BattleMapLogic.h"
#include<vector>
USING_NS_CC;
const float LABEL_OFFSET_Y = 20.0f;  // 标签在建筑上方的偏移量
const float LABEL_FONT_SIZE = 20.0f; // 字体大小


Building::Building() :
    _turf(nullptr),
    _buildingSprite(nullptr),
    _isSelected(false),
    _isDragging(false),
    _buildingScaleRatio(0.7f),
    _debugDrawNode(nullptr),
    _diamondWidthRatio(0.7f),
    _diamondHeightRatio(0.7f),
    _size(3),
    _infoLabel(nullptr),      // 初始化标签
    _buildingName("Building"),
    _level(1),
    _HP(0),
    _cost(0),
    _maxLevel(3)
{
}

Building* Building::create(const std::string& buildingFile,
    bool isHomeTown,
    const std::string turfFile,
    float buildingScale)
{
    Building* ret = new (std::nothrow) Building();
    ret->setIsHownTown(isHomeTown);
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Building::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    if (!Node::init()) {
        return false;
    }

    _buildingScaleRatio = buildingScale;
    std::string actualTurfFile = turfFile.empty() ? "grass.png" : turfFile;
    _turf = Sprite::create(actualTurfFile);

    if (!_turf) {
        return false;
    }

    this->addChild(_turf, 0);
    _turf->setPosition(Vec2::ZERO);
    _turf->setAnchorPoint(Vec2(0.5f, 0.5f));

    setBuildingTileSize();

    if (!buildingFile.empty()) {
        _buildingSprite = Sprite::create(buildingFile);
        if (_buildingSprite) {
            this->addChild(_buildingSprite, 1);
        }
    }

    setupBuildingOnTurf();
    this->setAnchorPoint(Vec2(0.5f, 0.5f));

    auto listener = EventListenerMouse::create();
    listener->onMouseDown = CC_CALLBACK_1(Building::onBuildingMouseDown, this);
    listener->onMouseUp = CC_CALLBACK_1(Building::onBuildingMouseUp, this);
    listener->onMouseMove = CC_CALLBACK_1(Building::onBuildingMouseMove, this);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    //血条
    this->initHPBar(false);

    return true;
}

void Building::setupBuildingOnTurf()
{
    if (!_buildingSprite || !_turf) {
        return;
    }

    Size turfSize = _turf->getContentSize() * _turf->getScale();
    Size buildingSize = _buildingSprite->getContentSize();

    float targetWidth = turfSize.width * _buildingScaleRatio;
    float targetHeight = turfSize.height * _buildingScaleRatio;

    float scaleX = targetWidth / buildingSize.width;
    float scaleY = targetHeight / buildingSize.height;
    float scale = MIN(scaleX, scaleY);

    _buildingSprite->setScale(scale);
    _buildingSprite->setPosition(Vec2(0, 10));
    _buildingSprite->setAnchorPoint(Vec2(0.5f, 0.5f));


}

void Building::setBuildingScale(float scale)
{
    if (scale > 0 && scale <= 1.0f) {
        _buildingScaleRatio = scale;
        setupBuildingOnTurf();
    }
}

void Building::setTurfScale(float scale)
{
    if (_turf && scale > 0) {
        _turf->setScale(scale);
        setupBuildingOnTurf();
    }
}

void Building::setBuildingTileSize()
{
    if (!_turf) return;
    float targetWidth;
    float targetHeight;
    if (!_isHomeTown) {
        targetWidth = BATTLE_MAP_TILE_W * _size;
        targetHeight = BATTLE_MAP_TILE_H * _size;
    }
    else {
        targetWidth = TILE_W * _size;
        targetHeight = TILE_H * _size;
    }



    Size buildingSize = _turf->getContentSize();

    float scaleX = targetWidth / buildingSize.width;
    float scaleY = targetHeight / buildingSize.height;

    _turf->setScale(MIN(scaleX, scaleY));
    _turf->setPosition(Vec2::ZERO);
    _turf->setAnchorPoint(Vec2(0.5f, 0.5f));
}


void Building::onBuildingMouseDown(Event* event)
{
    auto e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT)
        return;

    if (!isClickingInTurf(this->_parent, e))
        return;

    if (_isHomeTown) {
        // 通知 Camp：我要被选中了
        auto camp = dynamic_cast<Camp*>(Director::getInstance()->getRunningScene());
        if (camp) {
            camp->selectBuilding(this);
        }

        // 开始拖拽
        _isDragging = true;
        _lastMousePos = Vec2(e->getCursorX(), e->getCursorY());
        _dragStartPos = this->getPosition();
    }
    else {
        auto camp = dynamic_cast<BattleScene1*>(Director::getInstance()->getRunningScene());
        if (camp) {
            camp->selectBuilding(this);
        }
    }
    event->stopPropagation();
}



void Building::onBuildingMouseUp(Event* event)
{
    if (!_isDragging) return;

    _isDragging = false;

    Node* mapNode = this->getParent();
    if (mapNode) {
        snapToTile(mapNode, _size, _size);
    }

    event->stopPropagation();
}


void Building::onBuildingMouseMove(Event* event)
{
    if (!_isDragging) return;

    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    Vec2 delta = currentMousePos - _lastMousePos;

    Node* parent = this->getParent();
    float scale = parent ? parent->getScale() : 1.0f;
    Vec2 localDelta = delta / scale;

    this->setPosition(this->getPosition() + localDelta);
    _lastMousePos = currentMousePos;

    event->stopPropagation();
}

void Building::setTilePosition(Node* mapNode, float tileX, float tileY)
{
    if (!mapNode) return;

    Vec2 pos = tileToMapLocal(mapNode, tileX, tileY, _isHomeTown);
    this->setPosition(pos);
}

void Building::setBuildingSize(int size)
{
    if (size <= 0) return;

    _size = size;


    setBuildingTileSize();
}

void Building::drawDebugMapRange(Node* mapNode)
{
    if (!mapNode) return;

    // debug 节点挂在 mapNode 下
    auto debugNode = DrawNode::create();
    mapNode->addChild(debugNode, 9999);

    Size mapSize = mapNode->getContentSize();

    Vec2 mapCenter(mapSize.width * 0.5f + BATTLE_MAP_OFFSETX, mapSize.height * 0.5f + BATTLE_MAP_OFFSETY);


    // 遍历 tile 范围
    for (int x = -BATTLE_MAP_HALF; x < BATTLE_MAP_HALF; ++x)
    {
        for (int y = -BATTLE_MAP_HALF; y < BATTLE_MAP_HALF; ++y)
        {

            // tile 坐标 -> map 本地坐标 (等距投影)
            Vec2 pos;
            pos.x = (x - y) * (BATTLE_MAP_TILE_W / 2.0f);
            pos.y = (x + y) * (BATTLE_MAP_TILE_H / 2.0f);

            pos += mapCenter; // 平移到 map 中心

            debugNode->drawDot(pos, 2.0f, Color4F(0, 1, 0, 1)); // 亮绿色

        }
    }

    // 中心点
    debugNode->drawDot(mapCenter, 2.0f, Color4F::RED);
}


void Building::snapToTile(Node* mapNode, int buildingTileW, int buildingTileH)
{
    if (!mapNode) return;

    float deltaX, deltaY;
    if (buildingTileW % 2 == 0) {
        deltaX = -0.5;
    }
    else {
        deltaX = 0;
    }

    if (buildingTileH % 2 == 0) {
        deltaY = -0.5;
    }
    else {
        deltaY = 0;
    }


    //  建筑中心在地图本地坐标
    Vec2 worldCenter = this->convertToWorldSpace(Vec2::ZERO);
    Vec2 mapLocal = mapNode->convertToNodeSpace(worldCenter);

    Vec2 tile = mapLocalToTile(mapNode, mapLocal, true);

    // 对齐到最近 tile 网格（根据建筑占用宽高）
    float snappedTileX = round((tile.x / buildingTileW) * buildingTileW);
    float snappedTileY = round((tile.y / buildingTileH) * buildingTileH);

    snappedTileX += deltaX;
    snappedTileY += deltaY;

    if (snappedTileX <= -HALF || snappedTileX >= HALF - 1 || snappedTileY <= -HALF || snappedTileY >= HALF - 1) {
        this->setPosition(_dragStartPos);
        return;
    }

    setTilePosition(mapNode, snappedTileX, snappedTileY);
}


bool Building::isCenterInsideMap(Node* mapNode) const
{
    if (!mapNode) return false;

    // 1. Building 中心 → 世界
    Vec2 worldCenter = this->convertToWorldSpace(Vec2::ZERO);

    // 2. 世界 → 地图本地（此时原点在贴图左下角）
    Vec2 mapLocal = mapNode->convertToNodeSpace(worldCenter);

    Size size = mapNode->getContentSize();

    Vec2 mapCenter(size.width / 2 + offsetX, size.height / 2 + offsetY);
    Vec2 centered = mapLocal - mapCenter;


    // 4. 世界 → tile（用“中心化后的坐标”）
    float tileX = (centered.x / (TILE_W / 2.0f) +
        centered.y / (TILE_H / 2.0f)) * 0.5f;

    float tileY = (centered.y / (TILE_H / 2.0f) -
        centered.x / (TILE_W / 2.0f)) * 0.5f;

    // 5. 判断是否在 42×42 菱形范围内
    return
        tileX >= -HALF && tileX < HALF &&
        tileY >= -HALF && tileY < HALF;
}


bool Building::isClickingInTurf(Node* mapNode, cocos2d::EventMouse* e)
{
    if (!_turf) return false;


    //  建筑中心在地图本地坐标
    Vec2 worldCenter = this->convertToWorldSpace(Vec2::ZERO);
    Vec2 mapLocal = mapNode->convertToNodeSpace(worldCenter);

    Vec2 tile1 = mapLocalToTile(mapNode, mapLocal, _isHomeTown);

    Size mapSize = mapNode->getContentSize();
    Vec2 mapCenter(mapSize.width / 2 + offsetX, mapSize.height / 2 + offsetY);
    Vec2 centered = mapLocal - mapCenter;

    // 等距投影 → tile 坐标
    float tileX = (centered.x / (TILE_W / 2) + centered.y / (TILE_H / 2)) * 0.5f;
    float tileY = (centered.y / (TILE_H / 2) - centered.x / (TILE_W / 2)) * 0.5f;




    // 1. 获取点击点的世界坐标
    Vec2 clickPointWorld2 = Vec2(e->getCursorX(), e->getCursorY());


    Vec2 mapLocal2 = mapNode->convertToNodeSpace(clickPointWorld2);


    Vec2 tile2 = mapLocalToTile(mapNode, mapLocal2, _isHomeTown);

    Size mapSize2 = mapNode->getContentSize();

    Vec2 mapCenter2(mapSize2.width / 2 + offsetX, mapSize2.height / 2 + offsetY);
    Vec2 centered2 = mapLocal2 - mapCenter2;

    // 等距投影 → tile 坐标
    float tileX2 = (centered2.x / (TILE_W / 2) + centered2.y / (TILE_H / 2)) * 0.5f;
    float tileY2 = (centered2.y / (TILE_H / 2) - centered2.x / (TILE_W / 2)) * 0.5f;

    if (abs(tile1.x - tile2.x) < (_size / 2.0f) && abs(tile1.y - tile2.y) < (_size / 2.0f)) {
        return true;
    }
    else {
        return false;
    }
}


// ========== 新增：设置建筑名称 ==========
void Building::setBuildingName(const std::string& name)
{
    _buildingName = name;
    if (_isSelected && _infoLabel) {
        updateInfoLabel(); // 如果当前已选中，立即更新标签
    }
}

// ========== 新增：设置等级 ==========
void Building::setLevel(int level)
{
    if (level < 1) level = 1;
    if (level > _maxLevel) level = _maxLevel;

    if (_level != level) {
        _level = level;

		_HP = _upgradeSprites[_level-1]._hp;
		_cost = _upgradeSprites[_level - 1]._upgradeCost;
        // 如果有对应等级的图片，就更换
        auto it = _upgradeSprites.find(_level);
        if (it != _upgradeSprites.end() && !it->second.spriteFile.empty()) {
            changeBuildingSprite(it->second.spriteFile);
        }
        updateInfoLabel();
    }
}
// 控制标签显示
void Building::setShowInfoLabel(bool show)
{
    if (show) {
        showInfoLabel();
    }
    else {
        hideInfoLabel();
    }
}

// 创建信息标签
void Building::createInfoLabel()
{
    if (_infoLabel) {
        _infoLabel->removeFromParent();
        _infoLabel = nullptr;
    }

    // 创建标签
    _infoLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", LABEL_FONT_SIZE);
    if (!_infoLabel) {
        // 如果TTF字体不存在，使用系统字体
        _infoLabel = Label::createWithSystemFont("", "Arial", LABEL_FONT_SIZE);
    }

    if (_infoLabel) {
        this->addChild(_infoLabel, 100); // 较高的z-order确保显示在最前面

        // 设置标签样式
        _infoLabel->setTextColor(Color4B::WHITE);
        _infoLabel->enableOutline(Color4B::BLACK, 2); // 黑色描边，提高可读性
        _infoLabel->setAlignment(TextHAlignment::CENTER);

        // 初始设置为隐藏
        _infoLabel->setOpacity(0);
        _infoLabel->setVisible(false);
    }
}


// 显示信息标签
void Building::showInfoLabel()
{
    if (!_infoLabel) {
        createInfoLabel();
    }

    if (_infoLabel) {
        // 更新标签内容
        updateInfoLabel();

        // 设置标签位置（建筑上方）
        float labelY = _turf ? (_turf->getContentSize().height * _turf->getScaleY() / 2 + LABEL_OFFSET_Y) : LABEL_OFFSET_Y;
        _infoLabel->setPosition(Vec2(0, labelY));

        // 显示动画
        _infoLabel->setVisible(true);
        _infoLabel->stopAllActions();

        auto fadeIn = FadeIn::create(0.2f);
        auto scaleUp = ScaleTo::create(0.2f, 1.2f);
        auto scaleDown = ScaleTo::create(0.1f, 1.0f);
        auto spawn = Spawn::create(fadeIn, scaleUp, nullptr);
        _infoLabel->runAction(Sequence::create(spawn, scaleDown, nullptr));
    }
}

// 隐藏信息标签
void Building::hideInfoLabel()
{
    if (_infoLabel && _infoLabel->isVisible()) {
        _infoLabel->stopAllActions();

        auto fadeOut = FadeOut::create(0.2f);
        auto scaleDown = ScaleTo::create(0.2f, 0.8f);
        auto spawn = Spawn::create(fadeOut, scaleDown, nullptr);
        auto hide = CallFunc::create([this]() {
            if (_infoLabel) {
                _infoLabel->setVisible(false);
            }
            });

        _infoLabel->runAction(Sequence::create(spawn, hide, nullptr));
    }
}

// 更新标签内容
void Building::updateInfoLabel()
{
    if (!_infoLabel) return;

    // 构建显示文本：建筑名称 + 等级
    std::string displayText = _buildingName + " lv." + std::to_string(_level);
    _infoLabel->setString(displayText);
}

void Building::setSelected(bool selected)
{
    _isSelected = selected;

    if (selected) {
        if (_turf) _turf->setColor(Color3B(180, 180, 180));
        if (_buildingSprite) _buildingSprite->setColor(Color3B(220, 220, 220));

        this->runAction(ScaleTo::create(0.1f, this->getScale() * 1.05f));
        showInfoLabel();
        if (_isHomeTown) {
            if (_actionBar == nullptr) {
                _actionBar = BuildingActionBar::create();
                _actionBar->setVisible(false);
                auto scene = Director::getInstance()->getRunningScene();
                scene->addChild(_actionBar, 1000);
                
            }
            // 设置回调（使用lambda捕获this）
            _actionBar->setCallbacks(
                [this]() { this->onInfoButtonClicked(); },
                [this]() { this->onUpgradeButtonClicked(); }
            );

            // 显示操作栏
            _actionBar->showForBuilding(this);
        }

    }
    else {
        if (_turf) _turf->setColor(Color3B::WHITE);
        if (_buildingSprite) _buildingSprite->setColor(Color3B::WHITE);

        this->runAction(ScaleTo::create(0.1f, this->getScale() / 1.05f));
        hideInfoLabel();
        if (_isHomeTown) {
            _actionBar->hide();
        }

    }
}



// 按钮回调
void Building::onInfoButtonClicked()
{
    CCLOG("信息按钮: %s", _buildingName.c_str());
}


void Building::onUpgradeButtonClicked()
{
    CCLOG("升级按钮: %s Lv.%d", _buildingName.c_str(), _level);
    upgrade();  // 直接调用升级
}

// ========== 设置升级图片 ==========
void Building::setUpgradeSprite(int level, const std::string& spriteFile)
{
    if (level < 1 || level > _maxLevel) return;
    _upgradeSprites[level].spriteFile = spriteFile;
}

void Building::upgrade()
{
    if (_level >= _maxLevel) {

        return;
    }

    int oldLevel = _level;
    _level++;

    CCLOG("建筑 %s 从 %d 级升级到 %d 级",
        _buildingName.c_str(), oldLevel, _level);

    // 更换图片
    auto it = _upgradeSprites.find(_level);
    if (it != _upgradeSprites.end() && !it->second.spriteFile.empty()) {
        changeBuildingSprite(it->second.spriteFile);
    }

    _HP = _upgradeSprites[_level]._hp;
    _cost = _upgradeSprites[_level]._upgradeCost;

    // 更新标签
    if (_isSelected && _infoLabel) {
        updateInfoLabel();
    }
    // 假设升级后最大血量变化
    _HP = _upgradeSprites[_level]._hp;
    _currentHP = _HP; // 升级通常回复满血

    updateHPBar();

    // 播放升级特效
    playUpgradeEffect();
}

// ========== 新增：更换建筑精灵（保持草皮不变）==========
void Building::changeBuildingSprite(const std::string& newSpriteFile)
{
    if (!_turf) return;  // 必须有草皮

    // 移除旧的建筑精灵
    if (_buildingSprite) {
        this->removeChild(_buildingSprite);
        _buildingSprite = nullptr;
    }

    // 创建新的建筑精灵
    _buildingSprite = Sprite::create(newSpriteFile);
    if (!_buildingSprite) {
        CCLOG("错误：无法加载升级图片 %s", newSpriteFile.c_str());
        return;
    }

    // 添加到草皮上（继承你的原有结构）
    this->addChild(_buildingSprite, 1);  // z-order: 1，在草皮上面

    // 保持原有设置：位置和锚点
    _buildingSprite->setPosition(Vec2::ZERO);
    _buildingSprite->setAnchorPoint(Vec2(0.5f, 0.5f));

    // 调用原有的设置方法，保持与草皮的适配
    setupBuildingOnTurf();

    // 如果当前是选中状态，保持选中颜色
    if (_isSelected) {
        _buildingSprite->setColor(Color3B(220, 220, 220));
    }

    CCLOG("建筑 %s 更换为图片: %s", _buildingName.c_str(), newSpriteFile.c_str());
}

// ========== 升级特效 ==========
void Building::playUpgradeEffect()
{
    // 金色闪烁
    if (_buildingSprite) {
        auto tintToGold = TintTo::create(0.1f, Color3B(255, 215, 0));
        auto tintBack = TintTo::create(0.1f, Color3B::WHITE);
        _buildingSprite->runAction(Sequence::create(tintToGold, tintBack, nullptr));
    }
}

///////////////////////////
void Building::occupyTiles(float tileX, float tileY)
{
    _tileX = tileX;
    _tileY = tileY;

    int startX = tileX - _size / 2;
    int startY = tileY - _size / 2;

    for (int x = 0; x < _size; x++)
        for (int y = 0; y < _size; y++)
            occupyTile(startX + x, startY + y);
}

std::vector<cocos2d::Vec2> Building::getAttackTiles()
{
    std::vector<cocos2d::Vec2> result;

    int left = _tileX - _size / 2;
    int right = _tileX + _size / 2;
    int bottom = _tileY - _size / 2;
    int top = _tileY + _size / 2;

    for (int x = left - 1; x <= right + 1; x++)
    {
        for (int y = bottom - 1; y <= top + 1; y++)
        {
            if (isWalkable(x, y))
                result.push_back(Vec2(x, y));
        }
    }
    return result;
}
//血条
void Building::initHPBar(bool immediateShow) {
    if (_hpBar) return;

    _currentHP = _HP; // 初始血量
    if (immediateShow) {
        // 1. 创建背景
        _hpBarBg = Sprite::create();
        _hpBarBg->setColor(Color3B(80, 80, 80));
        _hpBarBg->setTextureRect(Rect(0, 0, 64, 10));
        _hpBarBg->setAnchorPoint(Vec2(0.5f, 0.5f));

        // 2. 创建 LoadingBar
        _hpBar = ui::LoadingBar::create();
        _hpBar->loadTexture("hp_red.png");
        _hpBar->ignoreContentAdaptWithSize(false);
        _hpBar->setContentSize(Size(60, 8));
        _hpBar->setPercent(100);
        _hpBar->setDirection(ui::LoadingBar::Direction::LEFT);
        _hpBar->setAnchorPoint(Vec2(0.5f, 0.5f));

        // 3. 计算位置（在建筑上方）
        float barY = _turf ? (_turf->getContentSize().height * _turf->getScaleY() / 2 + 15) : 40.0f;
        Vec2 barPos = Vec2(0, barY);

        _hpBarBg->setPosition(barPos);
        _hpBar->setPosition(barPos);

        this->addChild(_hpBarBg, 98);
        this->addChild(_hpBar, 99);
    }
}
void Building::updateHPBar() {
    if (!_hpBar || _HP <= 0) return;

    // 计算百分比
    float percent = ((float)_currentHP / (float)_HP) * 100.0f;
    _hpBar->setPercent(std::max(0.0f, percent));

}
void Building::reduceHP(float damage) {
    if (!isAlive()) return;
    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;
    this->updateHPBar();
    if (_currentHP <= 0) {
        _currentHP = 0;
        // 建筑摧毁逻辑
        CCLOG("建筑 %s 已被摧毁！", _buildingName.c_str());
        // this->removeFromParent(); // 暂时注释，避免崩溃，可改为变灰
        this->onDestroyed();
    }
}
void Building::onDestroyed() {
    CCLOG("建筑 %s 已被摧毁！", _buildingName.c_str());

    // 1. 视觉效果：比如变灰、冒烟或者播放爆炸动画
    this->setColor(Color3B::GRAY);

    // 2. 移除血条
    if (_hpBar) _hpBar->setVisible(false);

    // 3. 从物理层或逻辑层移除（根据你的需求，如果不删除节点，至少要标记为不可攻击）
    // 如果是大本营死亡，可能触发游戏结束逻辑
    if (_buildingName == "TownHall") {
        // 可以在这里通知场景游戏结束
    }

    // 建议延迟一点时间再真正移除，或者留着残骸
    this->runAction(Sequence::create(
        FadeOut::create(2.0f),
        RemoveSelf::create(),
        nullptr
    ));
}
