#include "BattleScene1.h"
#include"Building.h"
#include"Cannon.h"
#include "ui/CocosGUI.h"
#include"TownHall.h"
#include"ResourceStorageBuilding.h"
#include"ResourceCollector.h"
#include"Army_Camp.h"
#include "people.h"

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
    initTouchListener();


    // 场景全局/地图事件监听器 (处理滚轮和非房子区域的拖动)
   // auto _mouseListener = EventListenerMouse::create();

    //_mouseListener->onMouseMove = CC_CALLBACK_1(BattleScene1::onMapMouseMove, this);
    //_mouseListener->onMouseUp = CC_CALLBACK_1(BattleScene1::onMapMouseUp, this);
    //_mouseListener->onMouseDown = CC_CALLBACK_1(BattleScene1::onMapMouseDown, this);
    //_mouseListener->onMouseScroll = CC_CALLBACK_1(BattleScene1::onMouseScroll, this);

    // 注册监听器 (处理滚轮和地图拖动)
    //_eventDispatcher->addEventListenerWithSceneGraphPriority(_mouseListener, this);

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
void BattleScene1::spawnSoldier(Soldier::Type type, Vec2 pos) {
    auto soldier = Soldier::create(type, pos, 1.0f);
    if (!soldier) return;

    // 设置士兵等级（核心新增逻辑）
    soldier->setLevel(getTempSoldierLevel(type));
    soldier->bindScene(this);
    this->addChild(soldier, 1);

    // 标记战斗已开始
    _isBattleStarted = true;

    // 保留原有移动和队列逻辑
    float distance = pos.distance(_attackTargetPos);
    float duration = distance / SOLDIER_SPEED;
    soldier->moveToTarget(_attackTargetPos, duration);

    soldier->onReachTarget = [this](Soldier* s) {
        // 先判断是否在攻击范围内
        if (isInAttackRange(s)) {
            addToAttackQueue(s); // 加入攻击队列
        }
        else {
            CCLOG("士兵未进入攻击范围，不加入攻击队列");
        }
        };

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
                _soldierQueue.pop();
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
    float distance = soldier->getPosition().distance(_attackTargetPos);
    return distance <= _attackRange;
}

// 士兵进入范围后加入攻击队列
void BattleScene1::addToAttackQueue(Soldier* soldier) {
    if (!soldier->isAlive() || soldier->getState() != Soldier::State::MOVING) return;

    soldier->setState(Soldier::State::WAITING); // 进入等待状态
    _attackQueue.push(soldier);
    CCLOG("士兵加入攻击队列，当前队列长度：%d", (int)_attackQueue.size());

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
