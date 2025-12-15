#include "Building.h"
#include"BuildingActionBar.h"
USING_NS_CC;
const float TILE_W = 65.55f;    // tile 宽
const float TILE_H = 49.1625f;  // tile 高
const int HALF = 21;            // 当前地图 tile 半径
const float LABEL_OFFSET_Y = 20.0f;  // 标签在建筑上方的偏移量
const float LABEL_FONT_SIZE = 45.0f; // 字体大小
// ========== 新增：按钮相关常量 ==========


// 地图图片偏移矫正
const float offsetX = 51.0f; 
const float offsetY = 156.0f; 
Building::Building() :
    _turf(nullptr),
    _buildingSprite(nullptr),
    _isSelected(false),
    _isDragging(false),
    _buildingScaleRatio(0.7f),
    _debugDrawNode(nullptr),
    _diamondWidthRatio(0.7f),
    _diamondHeightRatio(0.7f),
    _size(4),
    _level(1),
    _infoLabel(nullptr),      // 初始化标签
    _buildingName("Building") 
   
{
}

Building* Building::create(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    Building* ret = new (std::nothrow) Building();
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

    setBuildingTileSize(4, 4);

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
    _buildingSprite->setPosition(Vec2::ZERO);
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

void Building::setBuildingTileSize(int tileWidthCount, int tileHeightCount)
{
    if (!_turf) return;

    float targetWidth = TILE_W * tileWidthCount;
    float targetHeight = TILE_H * tileHeightCount;

    Size buildingSize = _turf->getContentSize();

    float scaleX = targetWidth / buildingSize.width;
    float scaleY = targetHeight / buildingSize.height;

    _turf->setScale(MIN(scaleX, scaleY));
    _turf->setPosition(Vec2::ZERO);
    _turf->setAnchorPoint(Vec2(0.5f, 0.5f));
}


void Building::onBuildingMouseDown(Event* event)
{
    if (_isSelected) {
        
        EventMouse* e = static_cast<EventMouse*>(event);
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT &&
            isClickingInTurf(this->_parent, e))
        {
            _isDragging = true;
            _lastMousePos = Vec2(e->getCursorX(), e->getCursorY());
            _dragStartPos = this->getPosition();
            
            event->stopPropagation();
        }
    }
    else {
        _isSelected = true;
        EventMouse* e = static_cast<EventMouse*>(event);
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT &&
            isClickingInTurf(this->_parent, e))
        {
            if (_turf) _turf->setColor(Color3B(180, 180, 180));
            if (_buildingSprite) _buildingSprite->setColor(Color3B(220, 220, 220));
            this->runAction(ScaleTo::create(0.1f, this->getScale() * 1.05f));

            showInfoLabel();
            showActionBar();
            event->stopPropagation();
        }
    }
}


void Building::onBuildingMouseUp(Event* event)
{
    if (_isDragging) {
        _isSelected = false;
        _isDragging = false;

        if (_turf) _turf->setColor(Color3B::WHITE);
        if (_buildingSprite) _buildingSprite->setColor(Color3B::WHITE);
        Node* mapNode = this->getParent();
        
        if (mapNode) {
            // 假设建筑占 4x4 tiles
            snapToTile(mapNode, 4, 4);
        }

        this->runAction(ScaleTo::create(0.1f, this->getScale() / 1.05f));
       
         hideInfoLabel();
         hideActionBar();
        event->stopPropagation();
        return;
    }
    
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



void Building::drawDebugMapRange(Node* mapNode)
{
    if (!mapNode) return;

    // debug 节点挂在 mapNode 下
    auto debugNode = DrawNode::create();
    mapNode->addChild(debugNode, 9999);

    Size mapSize = mapNode->getContentSize();

    Vec2 mapCenter(mapSize.width * 0.5f + offsetX, mapSize.height * 0.5f + offsetY);


    // 遍历 tile 范围
    for (int x = -HALF; x < HALF; ++x)
    {
        for (int y = -HALF; y < HALF; ++y)
        {
            // 使用 isCenterInsideMap 同样的判断逻辑
            if (x >= -HALF && x < HALF && y >= -HALF && y < HALF)
            {
                // tile 坐标 -> map 本地坐标 (等距投影)
                Vec2 pos;
                pos.x = (x - y) * (TILE_W / 2.0f);
                pos.y = (x + y) * (TILE_H / 2.0f);

                pos += mapCenter; // 平移到 map 中心

                debugNode->drawDot(pos, 3.0f, Color4F(0, 1, 0, 1)); // 亮绿色
            }
        }
    }

    // 中心点
    debugNode->drawDot(mapCenter, 3.0f, Color4F::RED);
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

    Size mapSize = mapNode->getContentSize();
    Vec2 mapCenter(mapSize.width / 2 + offsetX, mapSize.height / 2 + offsetY);
    Vec2 centered = mapLocal - mapCenter;

    // 等距投影 → tile 坐标
    float tileX = (centered.x / (TILE_W / 2) + centered.y / (TILE_H / 2)) * 0.5f;
    float tileY = (centered.y / (TILE_H / 2) - centered.x / (TILE_W / 2)) * 0.5f;

    // 对齐到最近 tile 网格（根据建筑占用宽高）
    float snappedTileX = round((tileX / buildingTileW) * buildingTileW );
    float snappedTileY = round((tileY / buildingTileH) * buildingTileH );

    snappedTileX += deltaX;
    snappedTileY += deltaY;

    if (snappedTileX <= -HALF || snappedTileX >= HALF-1 || snappedTileY <= -HALF || snappedTileY >= HALF-1) {
        this->setPosition(_dragStartPos);
        return;
    }

    // tile → 地图本地坐标
    Vec2 snappedCentered;
    snappedCentered.x = (snappedTileX - snappedTileY) * (TILE_W / 2.0f);
    snappedCentered.y = (snappedTileX + snappedTileY) * (TILE_H / 2.0f);

    Vec2 snappedPos = mapCenter + snappedCentered;

    // 更新建筑位置
    this->setPosition(snappedPos);
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

    Size mapSize = mapNode->getContentSize();
    Vec2 mapCenter(mapSize.width / 2 + offsetX, mapSize.height / 2 + offsetY);
    Vec2 centered = mapLocal - mapCenter;

    // 等距投影 → tile 坐标
    float tileX = (centered.x / (TILE_W / 2) + centered.y / (TILE_H / 2)) * 0.5f;
    float tileY = (centered.y / (TILE_H / 2) - centered.x / (TILE_W / 2)) * 0.5f;




    // 1. 获取点击点的世界坐标
    Vec2 clickPointWorld2 = Vec2(e->getCursorX(), e->getCursorY());

 
    Vec2 mapLocal2 = mapNode->convertToNodeSpace(clickPointWorld2);

    Size mapSize2 = mapNode->getContentSize();
    
    Vec2 mapCenter2(mapSize2.width / 2 + offsetX, mapSize2.height / 2 + offsetY);
    Vec2 centered2 = mapLocal2 - mapCenter2;

    // 等距投影 → tile 坐标
    float tileX2 = (centered2.x / (TILE_W / 2) + centered2.y / (TILE_H / 2)) * 0.5f;
    float tileY2 = (centered2.y / (TILE_H / 2) - centered2.x / (TILE_W / 2)) * 0.5f;

	if (abs(tileX - tileX2) < 2.0f && abs(tileY - tileY2) < 2.0f) {
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
    if (level > 10) level = 10; // 假设最大10级

    if (_level != level) {
        _level = level;
        if (_isSelected && _infoLabel) {
            updateInfoLabel(); // 如果当前已选中，立即更新标签
        }
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
    if (!selected) {
        // 取消选择时恢复颜色
        if (_turf) _turf->setColor(Color3B::WHITE);
        if (_buildingSprite) _buildingSprite->setColor(Color3B::WHITE);

        // 恢复缩放
        this->runAction(ScaleTo::create(0.1f, this->getScale() / 1.05f));

        // 隐藏标签
        hideInfoLabel();
    }
}

// 显示操作栏 
void Building::showActionBar()
{
    auto actionBar = BuildingActionBar::getInstance();
    if (actionBar) {
        // 设置回调（使用lambda捕获this）
        actionBar->setCallbacks(
            [this]() { this->onInfoButtonClicked(); },
            [this]() { this->onUpgradeButtonClicked(); }
        );

        // 显示操作栏
        actionBar->showForBuilding(this);
    }
}

//隐藏操作栏（静态方法）
void Building::hideActionBar()
{
    auto actionBar = BuildingActionBar::getInstance();
    if (actionBar) {
        actionBar->hide();
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

    if (_level < 10) {
        _level++;
        updateInfoLabel();

        // 升级动画
        auto scaleUp = ScaleTo::create(0.1f, this->getScale() * 1.1f);
        auto scaleDown = ScaleTo::create(0.1f, this->getScale());
        this->runAction(Sequence::create(scaleUp, scaleDown, nullptr));

        CCLOG("%s 升级到 Lv.%d", _buildingName.c_str(), _level);
    }
}
