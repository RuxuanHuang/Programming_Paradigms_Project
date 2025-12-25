#include "BattleScene1.h"
#include"Building.h"
#include"Cannon.h"
#include "ui/CocosGUI.h"
#include"TownHall.h"
#include"ResourceStorageBuilding.h"
#include"ResourceCollector.h"
#include"Army_Camp.h"
#include "people.h"
#include"BattleMapLogic.h"

USING_NS_CC;

// 定义Tag常量
const int CAMP_SPRITE_TAG = 1;
// 缩放边界限制
const float MIN_SCALE = 1.1f;
const float MAX_SCALE = 3.0f;
const float SCALE_STEP = 0.1f; // 每次滚轮的缩放步长

// 初始化静态队列和标记
std::queue<Soldier*> BattleScene1::_destroyQueue;
bool BattleScene1::_isDestroying = false;


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

    for (int x = 0; x < MAP_SIZE; x++)
    {
        for (int y = 0; y < MAP_SIZE; y++)
        {
            g_map[x][y] = TileState::EMPTY;
        }
    }
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
    mapSprite->setScale(1.2);

    // 居中显示
    mapSprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(mapSprite, 0);


    // 初始化拖动状态
    _isMapDragging = false;


    auto townHall = TownHall::create("Town_Hall/Town_Hall3.png", false, "mud.png");
    //townHall->setLevel(3);
    townHall->setTilePosition(mapSprite, 0.5, 1.5);
    townHall->initHPBar(true); // <--- 添加这一行初始化血条
    mapSprite->addChild(townHall, 1);
    _buildings.push_back(townHall);  // <-- 添加这行
    // 新增：标记建筑占据的格子为不可通行
  // 假设建筑占据以(0.5,1.5)为中心的3x3区域
    for (int dx = -3; dx <= 1; dx++) {
        for (int dy = -3; dy <= 1; dy++) {
            int tileX = static_cast<int>(1.5 + dx);
            int tileY = static_cast<int>(2.5 + dy);
            occupyTile(tileX, tileY);
        }
    }
    //townHall->drawDebugMapRange(mapSprite);

    auto goldMine1 = GoldMine::create("Gold_Mine/Gold_Mine3.png", false, "mud.png");
    goldMine1->setTilePosition(mapSprite, -6, 4);
    goldMine1->initHPBar(true); // <--- 添加这一行初始化血条
    // 新增：标记建筑占据的格子为不可通行
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int tileX = static_cast<int>(-6 + dx);
            int tileY = static_cast<int>(4 + dy);
            occupyTile(tileX, tileY);
        }
    }
    mapSprite->addChild(goldMine1, 1);
    _buildings.push_back(goldMine1);

    auto goldMine2 = GoldMine::create("Gold_Mine/Gold_Mine3.png", false, "mud.png");
    goldMine2->setTilePosition(mapSprite, 7, 4);
    // 新增：标记建筑占据的格子为不可通行
    goldMine2->initHPBar(true); // <--- 添加这一行初始化血条
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int tileX = static_cast<int>(7 + dx);
            int tileY = static_cast<int>(4 + dy);
            occupyTile(tileX, tileY);
        }
    }
    mapSprite->addChild(goldMine2, 1);
    _buildings.push_back(goldMine2);

    auto ElixirStorage1 = ElixirStorage::create("Elixir_Storage/Elixir_Storage2.png", false, "mud.png");
    ElixirStorage1->setTilePosition(mapSprite, -6, 1);
    ElixirStorage1->initHPBar(true); // <--- 添加这一行初始化血条
    // 新增：标记建筑占据的格子为不可通行
  // 假设建筑占据以(0.5,1.5)为中心的3x3区域
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int tileX = static_cast<int>(-6 + dx);
            int tileY = static_cast<int>(1 + dy);
            occupyTile(tileX, tileY);
        }
    }
    mapSprite->addChild(ElixirStorage1, 1);
    _buildings.push_back(ElixirStorage1);

    auto ElixirStorage2 = ElixirStorage::create("Elixir_Storage/Elixir_Storage2.png", false, "mud.png");
    ElixirStorage2->setTilePosition(mapSprite, 7, 1);
    ElixirStorage2->initHPBar(true); // <--- 添加这一行初始化血条
    // 新增：标记建筑占据的格子为不可通行
  // 假设建筑占据以(0.5,1.5)为中心的3x3区域
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int tileX = static_cast<int>(7 + dx);
            int tileY = static_cast<int>(1 + dy);
            occupyTile(tileX, tileY);
        }
    }
    mapSprite->addChild(ElixirStorage2, 1);
    _buildings.push_back(ElixirStorage2);

    auto Cannon1 = Cannon::create("Cannon/Cannon2.png", false, "mud.png");
    Cannon1->setTilePosition(mapSprite, 0, -7);
    Cannon1->initHPBar(true); // <--- 添加这一行初始化血条
    Cannon1->startCombatAI();
    // 新增：标记建筑占据的格子为不可通行
  // 假设建筑占据以(0.5,1.5)为中心的3x3区域
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int tileX = static_cast<int>(0 + dx);
            int tileY = static_cast<int>(-7 + dy);
            occupyTile(tileX, tileY);
        }
    }
    mapSprite->addChild(Cannon1, 1);
    _buildings.push_back(Cannon1);

    auto Cannon2 = Cannon::create("Cannon/Cannon3.png", false, "mud.png");
    Cannon2->setTilePosition(mapSprite, 0, 9);
    Cannon2->initHPBar(true); // <--- 添加这一行初始化血条
    Cannon2->startCombatAI();
    // 新增：标记建筑占据的格子为不可通行
  // 假设建筑占据以(0.5,1.5)为中心的3x3区域
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int tileX = static_cast<int>(0 + dx);
            int tileY = static_cast<int>(9 + dy);
            occupyTile(tileX, tileY);
        }
    }
    mapSprite->addChild(Cannon2, 1);
    _buildings.push_back(Cannon2);

    // 设置默认目标建筑（比如第一个建筑）
    if (!_buildings.empty()) {
        _targetBuilding = _buildings[0];
        CCLOG("设置默认目标建筑：%p", _targetBuilding);
    }

    // 初始化战斗状态为未开始
    _isBattleStarted = false;
    // 加载人口分配数据（新增）
    loadMaxReleaseCountsFromPopulationManager();
    // 原目标点设置（保留你的逻辑）
    _attackTargetPos = Vec2(visibleSize.width - 500, visibleSize.height / 2);

    // 初始化选择界面和触摸监听（核心功能）
    initSoldierSelector();
    // 新增：创建人口按钮
    createPopulationButton();
    // 创建返回按钮（在适当位置添加这行调用）
    createBackButton();

    initTouchListener();


    // 场景全局/地图事件监听器 (处理滚轮和非房子区域的拖动)
   // auto _mouseListener = EventListenerMouse::create();

    //_mouseListener->onMouseMove = CC_CALLBACK_1(BattleScene1::onMapMouseMove, this);
    //_mouseListener->onMouseUp = CC_CALLBACK_1(BattleScene1::onMapMouseUp, this);
    //_mouseListener->onMouseDown = CC_CALLBACK_1(BattleScene1::onMapMouseDown, this);
    //_mouseListener->onMouseScroll = CC_CALLBACK_1(BattleScene1::onMouseScroll, this);

    // 注册监听器 (处理滚轮和地图拖动)
    //_eventDispatcher->addEventListenerWithSceneGraphPriority(_mouseListener, this);
    //visualizeBlockedTiles();
    return true;
}

/// /////////////////////
Vec2 worldPosToTile(Sprite* mapSprite, const Vec2& worldPos) {
    if (!mapSprite) return Vec2::ZERO;

    // 将世界坐标转换为相对于地图节点的局部坐标
    Vec2 localPos = mapSprite->convertToNodeSpace(worldPos);

    // 假设每个格子大小为64像素（根据实际调整）
    const float TILE_SIZE = 64.0f;

    // 计算格子坐标（四舍五入）
    float tileX = std::round(localPos.x / TILE_SIZE);
    float tileY = std::round(localPos.y / TILE_SIZE);

    return Vec2(tileX, tileY);
}

Vec2 tileToWorldPos(Sprite* mapSprite, const Vec2& tilePos) {
    if (!mapSprite) return Vec2::ZERO;

    const float TILE_SIZE = 64.0f;

    // 计算局部坐标
    Vec2 localPos(tilePos.x * TILE_SIZE, tilePos.y * TILE_SIZE);

    // 将局部坐标转换为世界坐标
    return mapSprite->convertToWorldSpace(localPos);
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


/////////////////////////////////////////////////////////////////////////////////


// 新增：检查是否可以进入人口分配场景
bool BattleScene1::canEnterPopulationScene() const {
    // 如果战斗已经开始，且已经释放了至少一个兵种，则不允许进入
    if (_isBattleStarted) {
        for (const auto& pair : _soldierReleaseCounts) {
            if (pair.second > 0) {
                return false;  // 已经释放了兵种，战斗已开始
            }
        }
    }
    return true;  // 战斗未开始，可以进入
}

// 新增：从PopulationManager加载最大释放数量
void BattleScene1::loadMaxReleaseCountsFromPopulationManager() {
    auto populationMgr = PopulationManager::getInstance();
    populationMgr->loadAssignment();  // 加载之前保存的分配

    // 获取每个兵种的分配数量并设置最大释放数量
    _maxReleaseCounts[Soldier::Type::INFANTRY] = populationMgr->getCharacterCount(CharacterType::INFANTRY);
    _maxReleaseCounts[Soldier::Type::ARCHER] = populationMgr->getCharacterCount(CharacterType::ARCHER);
    _maxReleaseCounts[Soldier::Type::CAVALRY] = populationMgr->getCharacterCount(CharacterType::CAVALRY);
    _maxReleaseCounts[Soldier::Type::MAGE] = populationMgr->getCharacterCount(CharacterType::MAGE);

    // 重置当前释放计数
    resetSoldierReleaseCounts();
}

// 新增：重置释放计数
void BattleScene1::resetSoldierReleaseCounts() {
    for (auto& pair : _soldierReleaseCounts) {
        pair.second = 0;
    }
    // 更新UI标签
    for (auto& pair : _countLabels) {
        updateCountLabel(pair.first);
    }
}
// 临时：根据兵种返回等级（后续替换为 TeamMate::getSoldierLevel(type)）
int BattleScene1::getTempSoldierLevel(Soldier::Type type) {
    switch (type) {
        case Soldier::Type::INFANTRY: return _infantryLevel;
        case Soldier::Type::ARCHER: return _archerLevel;
        case Soldier::Type::CAVALRY: return _cavalryLevel;
        case Soldier::Type::MAGE: return _mageLevel;
        default: return 1;
    }
}



// 新增：处理销毁队列（按顺序销毁士兵）
void BattleScene1::processDestroyQueue() {
    if (_destroyQueue.empty()) {
        _isDestroying = false; // 队列为空，标记为无销毁中
        return;
    }

    _isDestroying = true;
    Soldier* current = _destroyQueue.front(); // 取队首士兵

    // 3秒后销毁当前士兵，然后处理下一个
    current->runAction(Sequence::create(
        DelayTime::create(3.0f),
        CallFunc::create([this]() {
            // 销毁当前士兵
            _destroyQueue.front()->removeFromParentAndCleanup(true);
            _destroyQueue.pop();
            // 处理下一个士兵
            processDestroyQueue();
            }),
        nullptr
    ));
}

void BattleScene1::initSoldierSelector() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 底部选择栏背景
    auto bar = LayerColor::create(Color4B(50, 50, 50, 200), visibleSize.width, 100);
    bar->setPosition(origin);
    this->addChild(bar, 10);

    // 头像与类型映射
    std::vector<std::pair<Soldier::Type, std::string>> types = {
        {Soldier::Type::INFANTRY, "infantry_icon.png"},  // 步兵头像
        {Soldier::Type::ARCHER, "archer_icon.png"},      // 弓箭手头像
        {Soldier::Type::CAVALRY, "cavalry_icon.png"},    // 骑兵头像
        {Soldier::Type::MAGE, "mage_icon.png"}           // 法师头像
    };

    // 排列头像（横向分布）
    float startX = 500;  // 第一个头像的X坐标
    for (int i = 0; i < types.size(); i++) {
        auto type = types[i].first;
        auto iconPath = types[i].second;

        // 头像按钮
        auto btn = Button::create(iconPath);
        btn->setPosition(Vec2(startX + i * 200, 50));  // 间距200
        bar->addChild(btn);

        // 点击切换选中类型
        btn->addClickEventListener([this, type](Ref*) {
            _selectedType = type;
            });

        // 计数标签（头像下方） - 修改为显示 "已释放/最大数量"
        auto label = Label::createWithSystemFont("0/0", "Arial", 24);
        label->setPosition(Vec2(btn->getContentSize().width / 2, 0));
        btn->addChild(label);
        _countLabels[type] = label;
        _soldierReleaseCounts[type] = 0;  // 初始计数0
        updateCountLabel(type);

    }
}

// 在BattleScene1Scene.cpp中添加
// BattleScene1Scene.cpp - 修正createPopulationButton
void BattleScene1::createPopulationButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 不再定义局部变量，直接初始化类成员
    populationButton = Sprite::create("cat.png");
    if (populationButton == nullptr) {
        populationButton = Sprite::create();
        populationButton->setTextureRect(Rect(0, 0, 80, 80));
        populationButton->setColor(Color3B(255, 100, 100));  // 红色
    }

    // 按钮大小和位置（完全复用你的原有逻辑）
    float buttonSize = 60.0f;
    populationButton->setScale(buttonSize / populationButton->getContentSize().width);
    populationButton->setPosition(Vec2(origin.x + buttonSize / 2 + 20,
        origin.y + buttonSize / 2 + 20));

    // 触摸监听（保留你的原有逻辑，仅调整变量名）
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        // 改为访问类成员populationButton
        if (populationButton->getBoundingBox().containsPoint(touch->getLocation())) {
            populationButton->runAction(ScaleTo::create(0.1, 1.1));
            return true;
        }
        return false;
        };

    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        populationButton->runAction(ScaleTo::create(0.1, 1.0));
        if (populationButton->getBoundingBox().containsPoint(touch->getLocation())) {
            // 检查是否可以进入人口分配场景
            if (!canEnterPopulationScene()) {
                return;
            }
            // 获取当前已分配的人口总数
            auto populationMgr = PopulationManager::getInstance();
            populationMgr->loadAssignment();  // 加载上次保存的分配
            int totalPopulation = populationMgr->getTotalPopulation();

            // 如果还没有分配过，使用默认值
            if (totalPopulation <= 0) {
                totalPopulation = 20;  // 默认总人口
                populationMgr->initialize(totalPopulation);
            }

            CCLOG("打开人口分配场景，总人口: %d", totalPopulation);

            // 打开人口分配场景
            auto populationScene = PopulationScene::createScene(totalPopulation);
            Director::getInstance()->pushScene(populationScene);
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, populationButton);

    // 按钮文字（复用你的逻辑）
    auto buttonLabel = Label::createWithTTF("编队", "fonts/arial.ttf", 16);
    if (buttonLabel) {
        buttonLabel->setPosition(Vec2(populationButton->getContentSize().width / 2, -15));
        buttonLabel->setColor(Color3B::WHITE);
        populationButton->addChild(buttonLabel);
    }

    this->addChild(populationButton, 10);
}

// 在initTouchListener的触摸回调中调用spawnSoldier
// BattleScene1Scene.cpp - 修正initTouchListener
void BattleScene1::initTouchListener() {
    auto soldierListener = EventListenerTouchOneByOne::create();
    soldierListener->onTouchBegan = [this](Touch* touch, Event*) -> bool {
        // 1. 先判断是否点击人口按钮（访问类成员populationButton）
        if (populationButton && populationButton->getBoundingBox().containsPoint(touch->getLocation())) {
            return false; // 点击按钮则不处理士兵释放
        }

        // 2. 检查士兵释放上限（复用你的原有逻辑）
        if (_soldierReleaseCounts[_selectedType] >= _maxReleaseCounts[_selectedType]) {
            return false;
        }

        // 3. 创建士兵（复用你的原有逻辑）
        spawnSoldier(_selectedType, touch->getLocation());
        return true;
        };

    // 3.x 正确添加监听（无需setPriority）
    _eventDispatcher->addEventListenerWithSceneGraphPriority(soldierListener, this);
}

// 整合后的士兵创建函数（包含等级设置和原有逻辑）
void BattleScene1::spawnSoldier(Soldier::Type type, Vec2 worldPos) {
    auto soldier = Soldier::create(type, worldPos, 1.0f);
    if (!soldier) return;

    // 设置士兵等级和绑定场景
    soldier->setLevel(getTempSoldierLevel(type));
    soldier->bindScene(this);
    this->addChild(soldier, 1);
    // 将新士兵加入管理列表
    _activeSoldiers.push_back(soldier);
    // 标记战斗已开始
    _isBattleStarted = true;

    // ========== 关键修复：使用你的现有函数设置士兵的_tilePos ==========
    auto mapSprite = dynamic_cast<Sprite*>(getChildByTag(CAMP_SPRITE_TAG));
    if (mapSprite) {
        // 将世界坐标转换为地图局部坐标
        Vec2 mapLocal = mapSprite->convertToNodeSpace(worldPos);

        // 使用你的mapLocalToTile函数转换为格子坐标
        Vec2 startTile = mapLocalToTile(mapSprite, mapLocal, false); // false表示不是HomeTown

        // 设置士兵的格子位置
        soldier->setTilePosition(startTile);

        // 同时设置士兵的初始世界位置（直接使用传入的worldPos）
        soldier->setPosition(worldPos);

        CCLOG("士兵初始位置：世界(%.1f,%.1f) -> 地图局部(%.1f,%.1f) -> 格子(%.1f,%.1f)",
            worldPos.x, worldPos.y, mapLocal.x, mapLocal.y, startTile.x, startTile.y);
    }
    else {
        CCLOG("错误：无法获取地图节点！");
    }

    // ========== 让士兵朝建筑移动 ==========
    if (_targetBuilding) {
        CCLOG("士兵朝目标建筑移动");

        // 设置到达目标时的回调
        soldier->onReachTarget = [this](Soldier* s) {
            CCLOG("士兵到达目标位置");
            
        };
        soldier->setState(Soldier::State::WAITING);
        if (_targetBuilding && _targetBuilding->isAlive()) {
            soldier->startAttacking(_targetBuilding); // 启动 Soldier.cpp 里的 schedule 定时器
        }
        else {
            // 如果目标已经没了，直接处理下一个队列
            soldier->setState(Soldier::State::MOVING);
        }
        // 让士兵朝建筑移动（使用A*寻路）
        soldier->moveToBuilding(_targetBuilding);
    }
    else {
        // 如果没有目标建筑，使用原来的逻辑：移动到攻击点
        CCLOG("没有目标建筑，使用原攻击点移动逻辑");
        float distance = worldPos.distance(_attackTargetPos);
        float duration = distance / SOLDIER_SPEED;
        soldier->moveToTarget(_attackTargetPos, duration);

        soldier->onReachTarget = [this](Soldier* s) {
            if (isInAttackRange(s)) {
                addToAttackQueue(s);
            }
            };
    }

    // 更新计数
    _soldierReleaseCounts[type]++;
    updateCountLabel(type);
}

void BattleScene1::processSoldierQueue() {
    if (_soldierQueue.empty()) {  // 队列为空则退出
        _isProcessingQueue = false;
        return;
    }

    _isProcessingQueue = true;
    auto currentSoldier = _soldierQueue.front();  // 取队首士兵

    // 检查士兵是否存活，如果已死亡则直接处理下一个
    if (!currentSoldier->isAlive()) {
        _soldierQueue.pop();
        processSoldierQueue();
        return;
    }

    // 启动当前士兵受攻击
    currentSoldier->startTakeDamagePerSecond(_damagePerSec);
    CCLOG("开始攻击队列头部士兵，每秒扣血：%.1f", _damagePerSec);

    // 监听士兵死亡事件，死亡后处理下一个
    Director::getInstance()->getScheduler()->schedule(
        [this](float dt) {
            if (!_soldierQueue.empty() && !_soldierQueue.front()->isAlive()) {
                auto scene = dynamic_cast<BattleScene1*>(Director::getInstance()->getRunningScene());
                Soldier* targetSoldier = _soldierQueue.front();
                if (scene) {
                    scene->removeSoldierFromList(targetSoldier);
                }
                _soldierQueue.pop();
                targetSoldier->removeFromParentAndCleanup(true);
                this->_isProcessingQueue = false;

                this->processSoldierQueue(); // 处理下一个士兵
            }
        },
        this,
        0.5f,
        false,
        "SoldierQueueCheckTimer_" + std::to_string((long long)this)
    );
}

void BattleScene1::updateCountLabel(Soldier::Type type) {
    auto label = _countLabels[type];
    if (label) {
        // 显示“已释放/上限”
        label->setString(StringUtils::format("%d/%d",
            _soldierReleaseCounts[type], _maxReleaseCounts[type]));
    }
}

// 判断士兵是否进入攻击范围
bool BattleScene1::isInAttackRange(Soldier* soldier) {
    if (!soldier) return false;

    // 如果有目标建筑，使用建筑的攻击范围
    if (_targetBuilding) {
        // 方法1：检查士兵是否在建筑的攻击范围内
        // 可以根据需要实现不同的判断逻辑

        // 方法2：使用原有逻辑，但目标点设为建筑位置
        Vec2 attackCenter = _targetBuilding->getPosition();
        float distance = soldier->getPosition().distance(attackCenter);

        // 使用建筑的攻击范围或固定范围
        float buildingRange = _attackRange; // 可以改为建筑的特定范围
        return distance <= buildingRange;
    }
    else {
        // 如果没有目标建筑，使用原有逻辑
        float distance = soldier->getPosition().distance(_attackTargetPos);
        return distance <= _attackRange;
    }
}

// 士兵进入范围后加入攻击队列
void BattleScene1::addToAttackQueue(Soldier* soldier) {
    if (!soldier->isAlive() || soldier->getState() != Soldier::State::MOVING) return;

    soldier->setState(Soldier::State::WAITING); // 进入等待状态
    _attackQueue.push(soldier);
    CCLOG("士兵加入攻击队列，当前队列长度：%d", (int)_attackQueue.size());
    if (_targetBuilding && _targetBuilding->isAlive()) {
        soldier->startAttacking(_targetBuilding);
    }
    // 若队列未处理，立即启动第一个士兵受击
    if (!_isQueueProcessing) {
        processAttackQueue();
    }
}

void BattleScene1::processAttackQueue() {
    if (_attackQueue.empty()) {
        _isQueueProcessing = false;
        return;
    }

    _isQueueProcessing = true;

    // 清理队列中已死亡的士兵
    while (!_attackQueue.empty() && !_attackQueue.front()->isAlive()) {
        _attackQueue.pop();
    }

    if (_attackQueue.empty()) {
        _isQueueProcessing = false;
        return;
    }

    Soldier* frontSoldier = _attackQueue.front();

    // 启动攻击逻辑
    if (frontSoldier->getState() == Soldier::State::WAITING) {
        // 隐藏队列中其他所有士兵的血条
        std::queue<Soldier*> tempQueue = _attackQueue;
        tempQueue.pop(); // 跳过当前要攻击的士兵
        while (!tempQueue.empty()) {
            tempQueue.front()->hideHPBar();
            tempQueue.pop();
        }

        // 开始攻击并显示当前士兵的血条
        frontSoldier->startTakeDamagePerSecond(_damagePerSec);
        frontSoldier->showHPBar();
        CCLOG("开始攻击队列头部士兵，每秒扣血：%.1f", _damagePerSec);

        // 使用更可靠的检查方式
        Director::getInstance()->getScheduler()->schedule(
            [this](float dt) {
                if (_attackQueue.empty()) {
                    Director::getInstance()->getScheduler()->unschedule("QueueCheckTimer", this);
                    _isQueueProcessing = false;
                    return;
                }

                Soldier* front = _attackQueue.front();
                if (!front || !front->isAlive()) {
                    front->hideHPBar(); // 确保死亡后隐藏血条
                    _attackQueue.pop();
                    Director::getInstance()->getScheduler()->unschedule("QueueCheckTimer", this);
                    this->processAttackQueue(); // 处理下一个士兵
                }
            },
            this,
            0.05f,
            false,
            "QueueCheckTimer"
        );
    }
}

void BattleScene1::onEnter() {
    Scene::onEnter();

    // 每次进入场景时重新加载人口分配数据
    loadMaxReleaseCountsFromPopulationManager();
    CCLOG("战斗场景进入，重新加载人口分配");
}

// 新增：创建返回按钮的实现
void BattleScene1::createBackButton()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建返回按钮 - 使用战斗按钮的图片或自定义图片
    _backButton = Button::create("back.png", "", "");
    if (_backButton) {
        _backButton->setScale(0.3f);  // 根据需要调整缩放
        _backButton->setPosition(Vec2(origin.x + visibleSize.width * 0.95f,
            origin.y + visibleSize.height * 0.9f));  // 右上角位置

        // 添加点击事件：返回大本营场景
        _backButton->addClickEventListener(CC_CALLBACK_1(BattleScene1::backToCampCallback, this));

        // 添加返回文字标签（可选）
        auto backLabel = Label::createWithTTF("返回", "fonts/arial.ttf", 20);
        backLabel->setPosition(Vec2(_backButton->getContentSize().width / 2,
            -_backButton->getContentSize().height / 2 - 10));
        _backButton->addChild(backLabel);

        this->addChild(_backButton, 1000);  // 较高的层级确保按钮可见
    }
    else {
        // 如果图片加载失败，创建一个简单的按钮
        _backButton = Button::create();
        _backButton->setTitleText("返回营地");
        _backButton->setTitleFontSize(20);
        _backButton->setPosition(Vec2(origin.x + visibleSize.width - 80,
            origin.y + visibleSize.height - 50));
        _backButton->addClickEventListener(CC_CALLBACK_1(BattleScene1::backToCampCallback, this));
        this->addChild(_backButton, 1000);
    }
}

// 新增：返回按钮的回调函数
void BattleScene1::backToCampCallback(cocos2d::Ref* pSender)
{
    // 切换到营地场景
    auto campScene = Camp::createScene();

    Director::getInstance()->popScene();

    // 如果你想使用过渡效果，可以使用以下方式：
    // Director::getInstance()->replaceScene(TransitionFade::create(0.5f, campScene));

    CCLOG("从战斗场景返回到大本营场景");
}

void BattleScene1::visualizeBlockedTiles() {
    auto mapSprite = dynamic_cast<Sprite*>(getChildByTag(CAMP_SPRITE_TAG));
    if (!mapSprite) return;

    // 清除之前的标记
    auto debugLayer = this->getChildByName("DebugLayer");
    if (debugLayer) debugLayer->removeFromParent();

    // 创建新的调试层
    debugLayer = LayerColor::create(Color4B(0, 0, 0, 0));
    debugLayer->setName("DebugLayer");
    this->addChild(debugLayer, 9999);

    // 显示所有阻挡格子
    for (int x = -BATTLE_MAP_HALF; x < BATTLE_MAP_HALF; x++) {
        for (int y = -BATTLE_MAP_HALF; y < BATTLE_MAP_HALF; y++) {
            if (!isWalkable(x, y)) {
                // 将格子坐标转换为地图局部坐标
                Vec2 mapLocal = tileToMapLocal(mapSprite, x, y, false);

                // 将地图局部坐标转换为世界坐标
                Vec2 worldPos = mapSprite->convertToWorldSpace(mapLocal);

                // 创建红色方块标记阻挡
                auto marker = Sprite::create();
                marker->setTextureRect(Rect(0, 0, BATTLE_MAP_TILE_W, BATTLE_MAP_TILE_H));
                marker->setColor(Color3B(255, 0, 0));
                marker->setOpacity(100);
                marker->setPosition(worldPos);
                debugLayer->addChild(marker);

                CCLOG("阻挡格子(%d,%d) -> 地图局部(%.0f,%.0f) -> 世界坐标(%.0f,%.0f)",
                    x, y, mapLocal.x, mapLocal.y, worldPos.x, worldPos.y);
            }
        }
    }
    CCLOG("=== 可视化阻挡格子完成 ===");
}
void BattleScene1::removeSoldierFromList(Soldier* soldier) {
    // 使用 std::remove 将目标移动到末尾，然后 erase 掉
    auto it = std::remove(_activeSoldiers.begin(), _activeSoldiers.end(), soldier);
    if (it != _activeSoldiers.end()) {
        _activeSoldiers.erase(it, _activeSoldiers.end());
        CCLOG("士兵已从列表中剔除，剩余士兵数: %d", (int)_activeSoldiers.size());
    }
}