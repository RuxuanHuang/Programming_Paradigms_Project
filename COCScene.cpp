#include "COCScene.h"
// 初始化静态队列和标记
std::queue<Soldier*> COC::_destroyQueue;
bool COC::_isDestroying = false;

Scene* COC::createScene() {
    return COC::create();
}

bool COC::init() {
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    // --------------------------
    // 1. 添加背景图（核心代码）
    // --------------------------
    auto background = Sprite::create("background.png"); 
    if (background) {
        // 设置背景图位置为屏幕中心
        background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        // 按屏幕大小缩放背景图
        float scaleX = visibleSize.width / background->getContentSize().width;
        float scaleY = visibleSize.height / background->getContentSize().height;
        background->setScale(MAX(scaleX, scaleY));  // 按最大比例缩放，确保覆盖屏幕
        this->addChild(background, 0);  // 层级设为0（最低层，避免遮挡其他元素）
    }
    else {
        CCLOG("警告：背景图加载失败，请检查路径是否正确！");
    }

    // 原目标点设置（保留你的逻辑）
    _attackTargetPos = Vec2(visibleSize.width - 50, visibleSize.height / 2);

    // 初始化选择界面和触摸监听（核心功能）
    initSoldierSelector();
    initTouchListener();

    return true;
}

// 临时：根据兵种返回等级（后续替换为 TeamMate::getSoldierLevel(type)）
int COC::getTempSoldierLevel(Soldier::Type type) {
    switch (type) {
        case Soldier::Type::INFANTRY: return _infantryLevel;
        case Soldier::Type::ARCHER: return _archerLevel;
        case Soldier::Type::CAVALRY: return _cavalryLevel;
        case Soldier::Type::MAGE: return _mageLevel;
        default: return 1;
    }
}



// 新增：处理销毁队列（按顺序销毁士兵）
void COC::processDestroyQueue() {
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

void COC::initSoldierSelector() {
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

        // 计数标签（头像下方）
        auto label = Label::createWithSystemFont("0/0", "Arial", 24);
        label->setPosition(Vec2(btn->getContentSize().width / 2, 0));
        btn->addChild(label);
        _countLabels[type] = label;
        _soldierReleaseCounts[type] = 0;  // 初始计数0
        updateCountLabel(type);
    }
}

// 在initTouchListener的触摸回调中调用spawnSoldier
void COC::initTouchListener() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](Touch* touch, Event*) -> bool {
        auto currentType = _selectedType;

        // 检查是否达上限
        if (_soldierReleaseCounts[currentType] >= _maxReleaseCounts[currentType]) {
            return true;
        }

        // 调用新的士兵创建函数
        spawnSoldier(currentType, touch->getLocation());
        return true;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

// 整合后的士兵创建函数（包含等级设置和原有逻辑）
void COC::spawnSoldier(Soldier::Type type, Vec2 pos) {
    auto soldier = Soldier::create(type, pos, 2.0f);
    if (!soldier) return;

    // 设置士兵等级（核心新增逻辑）
    soldier->setLevel(getTempSoldierLevel(type));
    soldier->bindScene(this);
    this->addChild(soldier, 1);

    // 保留原有移动和队列逻辑
    float distance = pos.distance(_attackTargetPos);
    float duration = distance / SOLDIER_SPEED;
    soldier->moveToTarget(_attackTargetPos, duration);

    soldier->onReachTarget = [this](Soldier* s) {
        // 先判断是否在攻击范围内
        if (isInAttackRange(s)) {
            addToAttackQueue(s); // 加入攻击队列
        } else {
            CCLOG("士兵未进入攻击范围，不加入攻击队列");
        }
    };

    // 更新计数
    _soldierReleaseCounts[type]++;
    updateCountLabel(type);
}

void COC::processSoldierQueue() {
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

void COC::updateCountLabel(Soldier::Type type) {
    auto label = _countLabels[type];
    if (label) {
        // 显示“已释放/上限”
        label->setString(StringUtils::format("%d/%d",
            _soldierReleaseCounts[type], _maxReleaseCounts[type]));
    }
}

// 判断士兵是否进入攻击范围
bool COC::isInAttackRange(Soldier* soldier) {
    float distance = soldier->getPosition().distance(_attackTargetPos);
    return distance <= _attackRange;
}

// 士兵进入范围后加入攻击队列
void COC::addToAttackQueue(Soldier* soldier) {
    if (!soldier->isAlive() || soldier->getState() != Soldier::State::MOVING) return;

    soldier->setState(Soldier::State::WAITING); // 进入等待状态
    _attackQueue.push(soldier);
    CCLOG("士兵加入攻击队列，当前队列长度：%d", (int)_attackQueue.size());

    // 若队列未处理，立即启动第一个士兵受击
    if (!_isQueueProcessing) {
        processAttackQueue();
    }
}

void COC::processAttackQueue() {
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
        frontSoldier->startTakeDamagePerSecond(_damagePerSec);
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
                    _attackQueue.pop();
                    Director::getInstance()->getScheduler()->unschedule("QueueCheckTimer", this);
                    this->processAttackQueue();
                }
            },
            this,
            0.05f,
            false,
            "QueueCheckTimer"
        );
    }
}