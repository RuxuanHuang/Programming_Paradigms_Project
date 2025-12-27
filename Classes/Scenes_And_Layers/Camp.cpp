#include "Camp.h"
#include"Building.h"
#include"BuildersHut.h"
#include "BuildingActionBar.h"  
#include "ui/CocosGUI.h"
#include"Store.h"
#include"ResourceStorageBuilding.h"
#include"ResourceCollector.h"
#include"Army_Camp.h"
#include "BattleScene1.h" 
#include"Archer_Tower.h"
#include"Wall.h"
#include"Cannon.h"
#include "BuildingManager.h"
#include"resources.h"
#include"MapTools.h"
USING_NS_CC;

// 定义Tag常量
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

    ResourceManager* resourceMgr = ResourceManager::getInstance();

    // 创建资源显示UI（右上角）
    resourceMgr->createResourceDisplay(this);
    
   

    // 地图精灵初始化
    mapSprite = Sprite::create("others/Camp.png");
    if (mapSprite == nullptr)
    {
        problemLoading("'Campaaa.png'");
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


    auto BattleButton = Button::create("Buttons/FightingButton.png", "", "");
    if (BattleButton) {
        BattleButton->setScale(0.3f);  // 根据需要调整缩放
        BattleButton->setPosition(Vec2(visibleSize.width * 0.05f + origin.x,
            visibleSize.height * 0.1f + origin.y));  // 左下角位置

        // 添加点击事件：切换到战斗场景
        BattleButton->addClickEventListener([=](Ref*) {
            this->enterBattleScene();
            });

        this->addChild(BattleButton, 1000);
    }
    else {
        problemLoading("'FightingButton.png'");
    }
    BuildingManager* manager = BuildingManager::getInstance();
    manager->initialize();
    // ========== 创建大本营 ==========
    auto townHall = TownHall::create("Town_hall/Town_hall1.png");
    if (townHall == nullptr)
    {
        problemLoading("'Town_hall1.png' ");
    }
    else {
        townHall->setTilePosition(mapSprite, 0.5, 0.5);
        mapSprite->addChild(townHall, 1);
        _building = townHall;
		manager->addBuilding(townHall);
        townHall->setBuildingType("TownHall");
		setBuildingOccupy(townHall, true);
        // 显示网格范围
         //townHall->drawDebugMapRange(mapSprite);
    }
    
    auto BuilderHut = BuildersHut::create("others/Builders_Hut1.png");
    if (BuilderHut == nullptr)
    {
        problemLoading("'Builders_Hut1.png' ");
    }
    else {
        BuilderHut->setTilePosition(mapSprite, 5.5, 5.5);
        mapSprite->addChild(BuilderHut, 1);
        _building = BuilderHut;
        manager->addBuilding(BuilderHut);
        BuilderHut->setBuildingType("BuilderHut");
		setBuildingOccupy(BuilderHut, true);
    }
   

    auto StoreButton = Button::create("Buttons/StoreButton.png","","");
    if (StoreButton) {

        StoreButton->setScale(0.15f);
        StoreButton->setPosition(Vec2(visibleSize.width * 0.95f, 100));
        this->addChild(StoreButton, 100);

        StoreButton->addClickEventListener([=](Ref*) {
            openStore();
            });

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

void Camp::onMapMouseDown(Event* event)
{
    auto e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT)
        return;

    // 建筑优先
    if (_selectedBuilding && _selectedBuilding->isDragging()) {
        
        return;
    }

        

    _isMapDragging = true;
    _mapMoved = false;  // 重置
    _lastMapMousePos = Vec2(e->getCursorX(), e->getCursorY());
}




void Camp::onMapMouseUp(Event* event)
{
    auto e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT)
        return;

    _isMapDragging = false;

    // 只有“点击空白”才取消选中
    if (!_mapMoved) {
        if (_selectedBuilding) {
            // 有选中的建筑，检查其位置是否合法
            if (isBuildingPositionValid(_selectedBuilding)) {
                setBuildingOccupy(_selectedBuilding,true);
                clearSelection();
            }
            else {
                ;
                
            }
        }
        else {
            // 没有选中的建筑，就是普通的空白点击
            clearSelection();
        }
    }
}



void Camp::onMapMouseMove(Event* event)
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
// MARK: - 边界限制逻辑
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
void Camp::selectBuilding(Building* building)
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

        // 是否为大本营
        // 通过动态类型转换判断
        TownHall* townHall = dynamic_cast<TownHall*>(_selectedBuilding);
        if (townHall) {
            // 如果是大本营：
            // 1. 获取Building中的操作栏实例
            _globalActionBar = _selectedBuilding->_actionBar; 
            // 2. 设置换肤回调
            if (_globalActionBar) {
                _globalActionBar->setChangeSkinCallback([this]() {
                    this->changeMapSkin();
                    });
                // 3. 显示大本营专属操作栏（带切换外观按钮）
                _globalActionBar->showForTownHall(_selectedBuilding);
            }
        }
        else {
            // 如果是普通建筑,仅显示基础操作栏
            if (_selectedBuilding->_actionBar) {
                _selectedBuilding->_actionBar->showForBuilding(_selectedBuilding);
            }
        }
    }
}

void Camp::clearSelection()
{
    if (_selectedBuilding) {
        _selectedBuilding->setSelected(false);
        _selectedBuilding = nullptr;
    }
}

void Camp::changeMapSkin()
{
    auto mapSprite = dynamic_cast<Sprite*>(this->getChildByTag(CAMP_SPRITE_TAG));
    if (!mapSprite) return;

    // 切换地图皮肤
    if (_currentMapSkin == "others/Camp.png") {
        _currentMapSkin = "others/Campaaa.png"; 
    }
    else {
        _currentMapSkin = "others/Camp.png";
    }

    auto newTexture = Director::getInstance()->getTextureCache()->addImage(_currentMapSkin);
    if (newTexture) {
        mapSprite->setTexture(newTexture);
        // 保持原有缩放和位置
        float currentScale = mapSprite->getScale();
        mapSprite->setScale(currentScale);
        limitMapPos(mapSprite); // 防止位置偏移

        CCLOG("地图皮肤切换为: %s", _currentMapSkin.c_str());
    }
    else {
        CCLOG("加载地图皮肤失败: %s", _currentMapSkin.c_str());
    }
}


void Camp::enterBattleScene()
{
    // 切换到战斗场景
    auto battleScene = BattleScene1::createScene();  
    Director::getInstance()->pushScene(battleScene);
   
}

// 在大本营场景中
void Camp::openStore() {
    auto storeScene = Store::create();

    // 设置卡片选择回调
    storeScene->setCardSelectCallback([this](const std::string& cardName) {
        // 根据卡片名称创建对应的建筑
        this->createBuildingFromCard(cardName);
        });

    Director::getInstance()->pushScene(storeScene);
}

void Camp::createBuildingFromCard(const std::string& cardName) {
    BuildingManager* buildingManager = BuildingManager::getInstance();

    Building* newBuilding = buildingManager->createBuildingFromCard(cardName);
    if (!newBuilding) {
        CCLOG("错误: 创建建筑失败: %s", cardName.c_str());
        return;
    }
   
    if (newBuilding) {

        newBuilding->setTilePosition(mapSprite, 0, 0);
        // 添加到地图上
        mapSprite->addChild(newBuilding, 1);
        // 添加到存储向量中
        buildingManager->addBuilding(newBuilding);
        // 延迟一帧创建操作栏，确保已经完全回到了Camp场景
        this->scheduleOnce([this, newBuilding](float dt) {
            // 再次检查建筑是否还存在
            if (newBuilding && newBuilding->getParent()) {
                // 创建操作栏（如果还没有的话）
                if (newBuilding->_actionBar == nullptr) {
                    newBuilding->_actionBar = BuildingActionBar::create();
                    newBuilding->_actionBar->setVisible(false);

                    // 添加到Camp场景
                    this->addChild(newBuilding->_actionBar, 1000);

                    CCLOG("为建筑创建操作栏，父节点: %p", this);
                }

                // 选择新建筑
                this->selectBuilding(newBuilding);

                CCLOG("延迟选择建筑完成");
            }
            else {
                CCLOG("警告：建筑在延迟回调中无效");
            }
            }, 0.01f, "create_action_bar_" + cardName); // 使用很小的延迟
    }
    else {
        CCLOG("创建建筑失败，可能图片路径错误: %s", cardName.c_str());
    }
}


// 场景进入时
void Camp::onEnter()
{
    Scene::onEnter();

    // 创建资源显示
    ResourceManager::getInstance()->createResourceDisplay(this);
}

// 场景退出时
void Camp ::onExit()
{
    // 不清除显示，让ResourceManager只清空指针
    ResourceManager::getInstance()->removeResourceDisplay();

    Scene::onExit();
}

bool Camp::isBuildingPositionValid(Building* building)
{
    if (!building) return false;

    // 1. 获取建筑占用的所有网格坐标
    std::vector<Vec2> occupiedTiles = getBuildingOccupiedTiles(building);

    // 2. 检查每个网格是否被占用
    auto buildingmanager = BuildingManager::getInstance();
    for (const Vec2& tile : occupiedTiles) {
        if (!buildingmanager->tileIsEmpty(tile.x, tile.y)) {
            return false;
        }
    }
    return true;
}

std::vector<Vec2> Camp::getBuildingOccupiedTiles(Building* building)
{
    std::vector<Vec2> occupiedTiles;

    if (!building || !building->getParent()) {
        return occupiedTiles;
    }

    Vec2 centerTile = building->getTilePosition(mapSprite);

    


    // 获取建筑大小
    int size = building->getSize();

    // 计算起始网格坐标（假设建筑居中）
    int startX = static_cast<int>(centerTile.x) - size / 2;
    int startY = static_cast<int>(centerTile.y) - size / 2;

    if (size % 2 == 1) {
        int offset = size / 2; 

        for (int dy = -offset; dy <= offset; dy++) {
            for (int dx = -offset; dx <= offset; dx++) {
                int gridX = static_cast<int>(centerTile.x) + dx;
                int gridY = static_cast<int>(centerTile.y) + dy;

                occupiedTiles.push_back(Vec2(gridX + 21, gridY + 21));
            }
        }
    }
    else {
        int startX = static_cast<int>(floor(centerTile.x - (size / 2 - 0.5)));
        int startY = static_cast<int>(floor(centerTile.y - (size / 2 - 0.5)));

        for (int dy = 0; dy < size; dy++) {
            for (int dx = 0; dx < size; dx++) {
                int gridX = startX + dx;
                int gridY = startY + dy;

                occupiedTiles.push_back(Vec2(gridX + 21, gridY + 21));
            }
        }
    }

    return occupiedTiles;
}
void Camp::setBuildingOccupy(Building* building,bool occupy) {

    Vec2 centerTile = building->getTilePosition(mapSprite);

    // 获取建筑大小
    int size = building->getSize();

    // 计算起始网格坐标（假设建筑居中）
    int startX = static_cast<int>(centerTile.x) - size / 2;
    int startY = static_cast<int>(centerTile.y) - size / 2;
    auto buildingManager = BuildingManager::getInstance();
    if (size % 2 == 1) {
        int offset = size / 2;

        for (int dy = -offset; dy <= offset; dy++) {
            for (int dx = -offset; dx <= offset; dx++) {
                int gridX = static_cast<int>(centerTile.x) + dx;
                int gridY = static_cast<int>(centerTile.y) + dy;

				buildingManager->setGrid(gridX + 21, gridY + 21, occupy);
            }
        }
    }
    else {
        int startX = static_cast<int>(floor(centerTile.x - (size / 2 - 0.5)));
        int startY = static_cast<int>(floor(centerTile.y - (size / 2 - 0.5)));

        for (int dy = 0; dy < size; dy++) {
            for (int dx = 0; dx < size; dx++) {
                int gridX = startX + dx;
                int gridY = startY + dy;

                buildingManager->setGrid(gridX + 21, gridY + 21, occupy);
            }
        }
    }
}