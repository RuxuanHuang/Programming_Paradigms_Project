#include "Building.h"

USING_NS_CC;
const float TILE_W = 65.55f;    // tile 宽
const float TILE_H = 49.1625f;  // tile 高
const int HALF = 21;            // 当前地图 tile 半径
Building::Building() :
    _turf(nullptr),
    _buildingSprite(nullptr),
    _isDragging(false),
    _buildingScaleRatio(0.7f),
    _debugDrawNode(nullptr),
    _diamondWidthRatio(0.7f),
    _diamondHeightRatio(0.7f),
    _size(4),
    _level(1)
{
}

Building* Building::create(const std::string& buildingFile,
    const std::string& turfFile,
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
    const std::string& turfFile,
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

bool Building::pointInPolygon(const Vec2& point, Vec2 polygon[], int n)
{
    if (n < 3) return false;

    bool inside = false;

    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
            (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) /
                (polygon[j].y - polygon[i].y) + polygon[i].x)) {
            inside = !inside;
        }
    }

    return inside;
}

void Building::getDiamondVertices(Vec2 vertices[4])
{
    if (!_turf) return;

    vertices[0] = Vec2(2.65, 178);
    vertices[1] = Vec2(239, 0);
    vertices[2] = Vec2(-2.65, -178);
    vertices[3] = Vec2(-239, 0);
}

bool Building::isClickOnGreenDiamond(EventMouse* e)
{
    if (!_turf) return false;

    // 1. 获取点击点的世界坐标
    Vec2 clickPointWorld = Vec2(e->getCursorX(), e->getCursorY());

    // 2. 获取未缩放的硬编码顶点 (P_raw)
    Vec2 diamondRaw[4];
    getDiamondVertices(diamondRaw);

    // 3. 将 P_raw 转换为世界坐标系下的顶点 P_world
    Vec2 diamondWorld[4];

    // 我们必须将 P_raw 从 (未缩放，中心原点) 逐步转换为世界坐标。

    // 先转换为 Building 的本地坐标 (已缩放，中心原点)
    for (int i = 0; i < 4; i++) {
        // P_scaled = P_raw * Scale_turf (这是 _turf 自己的缩放)
        diamondWorld[i].x = diamondRaw[i].x * _turf->getScaleX();
        diamondWorld[i].y = diamondRaw[i].y * _turf->getScaleY();

        // 由于 _turf 在 Building (父节点) 的 (0, 0) 且锚点是 (0.5, 0.5)，
        // P_scaled 已经相对于 Building 锚点正确放置。
    }

    // 4. 将 Building 本地坐标转换为世界坐标
    // 这将考虑 Building 节点本身的 Position 和所有父节点的缩放。
    for (int i = 0; i < 4; i++) {
        diamondWorld[i] = this->convertToWorldSpace(diamondWorld[i]);
    }

    // 5. 判定
    // 比较： clickPointWorld (世界坐标) vs diamondWorld (世界坐标)
    return pointInPolygon(clickPointWorld, diamondWorld, 4);
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




void Building::showDiamondDebug(bool show)
{
    if (!_turf) return;

    if (show) {
        if (!_debugDrawNode) {
            _debugDrawNode = DrawNode::create();
            this->addChild(_debugDrawNode, 100);
        }

        updateDebugDisplay();
        _debugDrawNode->setVisible(true);
    }
    else if (_debugDrawNode) {
        _debugDrawNode->setVisible(false);
    }
}

void Building::setDiamondDebugParams(float widthRatio, float heightRatio)
{
    _diamondWidthRatio = widthRatio;
    _diamondHeightRatio = heightRatio;

    if (_debugDrawNode && _debugDrawNode->isVisible()) {
        updateDebugDisplay();
    }
}

void Building::updateDebugDisplay()
{
    if (_debugDrawNode && _turf) {
        _debugDrawNode->clear();

        Vec2 diamond[4];
        getDiamondVertices(diamond);

        for (int i = 0; i < 4; i++) {
            diamond[i].x *= _turf->getScaleX();
            diamond[i].y *= _turf->getScaleY();
        }

        // 原有：画菱形
        for (int i = 0; i < 4; i++) {
            _debugDrawNode->drawLine(
                diamond[i],
                diamond[(i + 1) % 4],
                Color4F::RED
            );
        }

        Color4F fillColor = Color4F::RED;
        fillColor.a = 0.3f;
        _debugDrawNode->drawSolidPoly(diamond, 4, fillColor);

        // 新增：只在 debug 里画中心点（草地 0,0）
        _debugDrawNode->drawDot(
            Vec2::ZERO,      // turf 的 (0,0)
            2.0f,
            Color4F::BLUE
        );
    }
}


void Building::onBuildingMouseDown(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);

    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT &&
        isClickingInTurf(this->_parent,e))
    {
        _isDragging = true;
        _lastMousePos = Vec2(e->getCursorX(), e->getCursorY());

        if (_turf) _turf->setColor(Color3B(180, 180, 180));
        if (_buildingSprite) _buildingSprite->setColor(Color3B(220, 220, 220));

        _dragStartPos = this->getPosition();
        this->runAction(ScaleTo::create(0.1f, this->getScale() * 1.05f));
        event->stopPropagation();
    }
}


void Building::onBuildingMouseUp(Event* event)
{
    if (_isDragging) {
        _isDragging = false;

        if (_turf) _turf->setColor(Color3B::WHITE);
        if (_buildingSprite) _buildingSprite->setColor(Color3B::WHITE);
        Node* mapNode = this->getParent();
        
        if (mapNode) {
            // 假设建筑占 4x4 tiles
            snapToTile(mapNode, 4, 4);
        }

        this->runAction(ScaleTo::create(0.1f, this->getScale() / 1.05f));
        event->stopPropagation();
    }
}

void Building::drawDebugMapRange(Node* mapNode)
{
    if (!mapNode) return;

    

    // debug 节点挂在 mapNode 下
    auto debugNode = DrawNode::create();
    mapNode->addChild(debugNode, 9999);

    Size mapSize = mapNode->getContentSize();

    //  使用和判定一致的偏移量
    float offsetX = 51.0f; // 与 isCenterInsideMap() 保持一致
    float offsetY = 156.0f;

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
    float offsetX = 51.0f;   // 与 isCenterInsideMap 保持一致
    float offsetY = 156.0f;
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





bool Building::isCenterInsideMap(Node* mapNode) const
{
    if (!mapNode) return false;

    // 1. Building 中心 → 世界
    Vec2 worldCenter = this->convertToWorldSpace(Vec2::ZERO);

    // 2. 世界 → 地图本地（此时原点在贴图左下角）
    Vec2 mapLocal = mapNode->convertToNodeSpace(worldCenter);

    Size size = mapNode->getContentSize();

    // ★ 3新增：根据图片实际偏移微调
    float offsetX = 51.0f; // X 方向偏移量，可正/负调整
    float offsetY = 156.0f; // Y 方向偏移量，可正/负调整

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
    float offsetX = 51.0f;   // 与 isCenterInsideMap 保持一致
    float offsetY = 156.0f;
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



void Building::updateBuildingAppearance() {
    if (_buildingSprite) {
        this->removeChild(_buildingSprite);
    }

    
    //_buildingSprite = Sprite::create(_levelSprites[_level - 1]);
    
    //this->addChild(_buildingSprite, 1);
    
}
