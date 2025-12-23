#include "BattleScene1.h"
#include"Building.h"
#include"Cannon.h"
#include "ui/CocosGUI.h"
#include"TownHall.h"
#include"ResourceStorageBuilding.h"
#include"ResourceCollector.h"
#include"Army_Camp.h"
#include"Wall.h"

USING_NS_CC;

// 定义Tag常量
const int CAMP_SPRITE_TAG = 1;
// 缩放边界限制
const float MIN_SCALE = 1.1f;
const float MAX_SCALE = 3.0f;
const float SCALE_STEP = 0.1f; // 每次滚轮的缩放步长

Scene* BattleScene1::createScene()
{
    return BattleScene1::create();

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

bool BattleScene1::init()
{
    if (!Scene::init())
    {
        return false;
    }    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();


    // 地图精灵初始化
    auto mapSprite = Sprite::create("others/BattleMap1.png");
    if (mapSprite == nullptr)
    {
        problemLoading("'BattleMap1.png'");
        return false;
    }

    mapSprite->setTag(CAMP_SPRITE_TAG);


    // 等比初始缩放
    float scaleX = visibleSize.width / mapSprite->getContentSize().width;
    float scaleY = visibleSize.height / mapSprite->getContentSize().height;
    float initScale = MIN(scaleX, scaleY);
    mapSprite->setScale(1.3);

    // 居中显示
    mapSprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(mapSprite, 0);


    // 初始化拖动状态
    _isMapDragging = false;


    auto townHall = TownHall::create("Town_Hall/Town_Hall3.png", false, "mud.png");
    townHall->setTilePosition(mapSprite, 0.5, 1.5);
    mapSprite->addChild(townHall, 1);
    townHall->drawDebugMapRange(mapSprite);

    auto goldMine1 = GoldMine::create("Gold_Mine/Gold_Mine3.png", false, "mud.png");
    goldMine1->setTilePosition(mapSprite, -6, 4);
    mapSprite->addChild(goldMine1, 1);

    auto goldMine2 = GoldMine::create("Gold_Mine/Gold_Mine3.png", false, "mud.png");
    goldMine2->setTilePosition(mapSprite, 7, 4);
    mapSprite->addChild(goldMine2, 1);

    auto ElixirStorage1 = ElixirStorage::create("Elixir_Storage/Elixir_Storage2.png", false, "mud.png");
    ElixirStorage1->setTilePosition(mapSprite, -6, 1);
    mapSprite->addChild(ElixirStorage1, 1);

    auto ElixirStorage2 = ElixirStorage::create("Elixir_Storage/Elixir_Storage2.png", false, "mud.png");
    ElixirStorage2->setTilePosition(mapSprite, 7, 1);
    mapSprite->addChild(ElixirStorage2, 1);

    auto Cannon1 = Cannon::create("Cannon/Cannon2.png", false, "mud.png");
    Cannon1->setTilePosition(mapSprite, 0, -7);
    mapSprite->addChild(Cannon1, 1);

    auto Cannon2 = Cannon::create("Cannon/Cannon3.png", false, "mud.png");
    Cannon2->setTilePosition(mapSprite, 0, 9);
    mapSprite->addChild(Cannon2, 1);




    // 场景全局/地图事件监听器 (处理滚轮和非房子区域的拖动)
    auto _mouseListener = EventListenerMouse::create();

    _mouseListener->onMouseMove = CC_CALLBACK_1(BattleScene1::onMapMouseMove, this);
   _mouseListener->onMouseUp = CC_CALLBACK_1(BattleScene1::onMapMouseUp, this);
    _mouseListener->onMouseDown = CC_CALLBACK_1(BattleScene1::onMapMouseDown, this);
    _mouseListener->onMouseScroll = CC_CALLBACK_1(BattleScene1::onMouseScroll, this);

    // 注册监听器 (处理滚轮和地图拖动)
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_mouseListener, this);

    return true;
}

void BattleScene1::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}

// ----------------------------------------------------------------------------------
// MARK: - 地图 (MapSprite) 拖动/缩放逻辑
// ----------------------------------------------------------------------------------

// 滚轮事件处理函数
void BattleScene1::onMouseScroll(Event* event)
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

void BattleScene1::onMapMouseDown(Event* event)
{
    auto e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT)
        return;

    // 建筑优先
    if (_selectedBuilding && _selectedBuilding->isDragging())
        return;

    _isMapDragging = true;
    _mapMoved = false;  // 重置
    _lastMapMousePos = Vec2(e->getCursorX(), e->getCursorY());
}




void BattleScene1::onMapMouseUp(Event* event)
{
    auto e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT)
        return;

    _isMapDragging = false;

    // 只有“点击空白”才取消选中
    if (!_mapMoved) {
        clearSelection();
    }
}



void BattleScene1::onMapMouseMove(Event* event)
{
    if (!_isMapDragging) return;

    if (_selectedBuilding && _selectedBuilding->isDragging())
        return;

    auto e = static_cast<EventMouse*>(event);
    Vec2 current(e->getCursorX(), e->getCursorY());
    Vec2 delta = current - _lastMapMousePos;

    // 只有真的移动了，才算拖拽
    if (delta.lengthSquared() > 1.0f) {
        _mapMoved = true;
    }

    auto mapSprite = dynamic_cast<Sprite*>(getChildByTag(CAMP_SPRITE_TAG));
    mapSprite->setPosition(mapSprite->getPosition() + delta);
    limitMapPos(mapSprite);

    _lastMapMousePos = current;
}




// ----------------------------------------------------------------------------------
// MARK: - 边界限制逻辑（保持不变）
// ----------------------------------------------------------------------------------

void BattleScene1::limitMapPos(Sprite* sprite)
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
void BattleScene1::selectBuilding(Building* building)
{
    if (_selectedBuilding == building)
        return;

    // 取消旧的选中状态
    if (_selectedBuilding) {
        _selectedBuilding->setSelected(false);
    }

    _selectedBuilding = building;

    if (_selectedBuilding) {
        // 标记建筑为选中
        _selectedBuilding->setSelected(true);
    }
}

void BattleScene1::clearSelection()
{
    if (_selectedBuilding) {
        _selectedBuilding->setSelected(false);
        _selectedBuilding = nullptr;
    }
}

