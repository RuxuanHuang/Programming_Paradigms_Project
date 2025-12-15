#include "Camp.h"
#include"Building.h"
#include "BuildingActionBar.h"  // 新增：包含操作栏头文件
#include "ui/CocosGUI.h"

USING_NS_CC;

// 定义精灵的Tag常量
const int CAMP_SPRITE_TAG = 1;
// 缩放边界限制
const float MIN_SCALE = 0.56f;
const float MAX_SCALE = 3.0f;
const float SCALE_STEP = 0.1f; // 每次滚轮的缩放步长

Scene* Camp::createScene()
{
    return Camp::create();
}

// 资源加载错误提示
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// ----------------------------------------------------------------------------------
// MARK: - Scene 初始化
// ----------------------------------------------------------------------------------

bool Camp::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ========== 新增：初始化建筑操作栏 ==========
    auto actionBar = BuildingActionBar::getInstance();
    this->addChild(actionBar, 1000); // 高z-order确保在最前面

   
 

    // 地图精灵初始化
    auto mapSprite = Sprite::create("Camp.png");
    if (mapSprite == nullptr)
    {
        problemLoading("'Camp.png'");
        return false;
    }

    mapSprite->setTag(CAMP_SPRITE_TAG);

    // 等比初始缩放
    float scaleX = visibleSize.width / mapSprite->getContentSize().width;
    float scaleY = visibleSize.height / mapSprite->getContentSize().height;
    float initScale = MIN(scaleX, scaleY);
    mapSprite->setScale(initScale);

    // 居中显示
    mapSprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(mapSprite, 0);


    // 初始化拖动状态
    _isMapDragging = false;
    // 移除：_isBuildingDragging = false; 

    // ========== 修改：创建大本营 ==========
    auto townHall = TownHall::create("Town_hall1.png");
    if (townHall == nullptr)
    {
        problemLoading("'Town_hall1.png' ");
    }
    else {
        // 重要：将大本营添加到地图精灵，而不是场景
        Vec2 mapCenter = Vec2(mapSprite->getContentSize().width / 2,
            mapSprite->getContentSize().height / 2);
        townHall->setPosition(mapCenter);
        mapSprite->addChild(townHall, 1);
        _townHall = townHall;

        // 调试：显示网格范围（可选）
        // townHall->drawDebugMapRange(mapSprite);
    }

    // 场景全局/地图事件监听器 (处理滚轮和非房子区域的拖动)
    auto _mouseListener = EventListenerMouse::create();

    _mouseListener->onMouseMove = CC_CALLBACK_1(Camp::onMapMouseMove, this);
    _mouseListener->onMouseUp = CC_CALLBACK_1(Camp::onMapMouseUp, this);
    _mouseListener->onMouseDown = CC_CALLBACK_1(Camp::onMapMouseDown, this);
    _mouseListener->onMouseScroll = CC_CALLBACK_1(Camp::onMouseScroll, this);

    // 注册监听器 (处理滚轮和地图拖动)
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_mouseListener, this);

    return true;
}

void Camp::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}

// ----------------------------------------------------------------------------------
// MARK: - 地图 (MapSprite) 拖动/缩放逻辑
// ----------------------------------------------------------------------------------

// 滚轮事件处理函数
void Camp::onMouseScroll(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    float scrollY = e->getScrollY();
    if (scrollY == 0) return;

    auto campSprite = dynamic_cast<Sprite*>(this->getChildByTag(CAMP_SPRITE_TAG));

    float currentScale = campSprite->getScale();
    float newScale = currentScale;

    if (scrollY > 0) {
        newScale = currentScale * (1 + SCALE_STEP);
        newScale = MIN(newScale, MAX_SCALE);
    }
    else {
        newScale = currentScale * (1 - SCALE_STEP);
        newScale = MAX(newScale, MIN_SCALE);
    }

    // 应用新缩放并限制位置
    campSprite->setScale(newScale);
    limitMapPos(campSprite);
}

// 地图鼠标按下：开始拖动地图（如果房子没有被拖动）
void Camp::onMapMouseDown(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        // ========== 修改：检查是否有任何建筑正在拖动 ==========
        auto campSprite = dynamic_cast<Sprite*>(this->getChildByTag(CAMP_SPRITE_TAG));
        bool anyBuildingDragging = false;

        if (campSprite) {
            for (auto child : campSprite->getChildren()) {
                Building* building = dynamic_cast<Building*>(child);
                if (building && building->isDragging()) {
                    anyBuildingDragging = true;
                    break;
                }
            }
        }

        // 如果有建筑正在拖动，不开始地图拖动
        if (anyBuildingDragging) {
            return;
        }

        _isMapDragging = true;
        _lastMapMousePos = Vec2(e->getCursorX(), e->getCursorY());
    }
}

// 地图鼠标抬起：清除地图拖动状态
void Camp::onMapMouseUp(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        // 清除地图拖动状态
        _isMapDragging = false;

        // 建筑的拖动状态由 Building::onBuildingMouseUp 负责清除
    }
}

// 地图鼠标移动：移动地图
void Camp::onMapMouseMove(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);

    // ========== 修改：检查是否有任何建筑正在拖动 ==========
    auto campSprite = dynamic_cast<Sprite*>(this->getChildByTag(CAMP_SPRITE_TAG));
    bool anyBuildingDragging = false;

    if (campSprite) {
        for (auto child : campSprite->getChildren()) {
            Building* building = dynamic_cast<Building*>(child);
            if (building && building->isDragging()) {
                anyBuildingDragging = true;
                break;
            }
        }
    }

    // 只有在地图拖动中，并且没有建筑被拖动时才执行
    if (!_isMapDragging || anyBuildingDragging) {
        return;
    }

    Vec2 _currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    Vec2 delta = _currentMousePos - _lastMapMousePos;
    auto mapSprite = dynamic_cast<Sprite*>(this->getChildByTag(CAMP_SPRITE_TAG));

    if (mapSprite) {
        Vec2 currentMapPos = mapSprite->getPosition();
        Vec2 newMapPos = currentMapPos + delta;

        mapSprite->setPosition(newMapPos);
        limitMapPos(mapSprite);
    }
    _lastMapMousePos = _currentMousePos;
}

// ----------------------------------------------------------------------------------
// MARK: - 边界限制逻辑（保持不变）
// ----------------------------------------------------------------------------------

void Camp::limitMapPos(Sprite* sprite)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 缩放后的地图尺寸的一半
    float halfScaledWidth = sprite->getContentSize().width * sprite->getScale() / 2.0f;
    float halfScaledHeight = sprite->getContentSize().height * sprite->getScale() / 2.0f;

    // 屏幕左下角和右上角的绝对坐标
    float screenLeft = origin.x;
    float screenBottom = origin.y;
    float screenRight = origin.x + visibleSize.width;
    float screenTop = origin.y + visibleSize.height;

    float minX, maxX;
    float minY, maxY;

    // --- X轴边界计算 ---
    if (halfScaledWidth * 2.0f <= visibleSize.width) {
        // 地图小于屏幕：中心点限制在地图能完全居中的范围内
        minX = screenLeft + halfScaledWidth;
        maxX = screenRight - halfScaledWidth;
    }
    else {
        // 地图大于屏幕：中心点限制在地图边缘能对齐屏幕边缘的范围内
        minX = screenRight - halfScaledWidth;
        maxX = screenLeft + halfScaledWidth;
    }

    // --- Y轴边界计算 ---
    if (halfScaledHeight * 2.0f <= visibleSize.height) {
        // 地图小于屏幕
        minY = screenBottom + halfScaledHeight;
        maxY = screenTop - halfScaledHeight;
    }
    else {
        // 地图大于屏幕
        minY = screenTop - halfScaledHeight;
        maxY = screenBottom + halfScaledHeight;
    }

    // 确保传递给 clampf 的是正确的最小值和最大值
    float finalMinX = MIN(minX, maxX);
    float finalMaxX = MAX(minX, maxX);
    float finalMinY = MIN(minY, maxY);
    float finalMaxY = MAX(minY, maxY);

    float currentX = sprite->getPositionX();
    float currentY = sprite->getPositionY();

    // 应用限制
    currentX = clampf(currentX, finalMinX, finalMaxX);
    currentY = clampf(currentY, finalMinY, finalMaxY);

    sprite->setPosition(currentX, currentY);
}