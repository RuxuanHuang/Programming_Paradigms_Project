#include "Soldier.h"
#include "BattleScene1.h"
#include"MapTools.h"
#include"Building.h"
#include "Player1.h"    // 步兵
#include "Archer.h"     // 弓箭手
#include "Cavalry.h"    // 骑兵
#include "Mage.h"       // 法师
#include "AStar.h"
using namespace cocos2d;
using namespace cocos2d::ui;

// 初始化LoadingBar血条（核心）
void Soldier::initHPBar() {
    // 创建LoadingBar
    _hpBar = LoadingBar::create();

    // 自定义图片
     _hpBar->loadTexture("hp_red.png");
     _hpBar->setContentSize(Size(_hpBarWidth, _hpBarHeight));
     _hpBar->setPercent(100);

    // 血条位置与属性
    _hpBar->setAnchorPoint(Vec2(0.5f, 0.0f));
    _hpBar->setPosition(Vec2(getContentSize().width / 2, getContentSize().height + 10));
    _hpBar->setDirection(LoadingBar::Direction::LEFT);

    // 添加背景条
    auto hpBg = Sprite::create();
    hpBg->setColor(Color3B(100, 100, 100));
    hpBg->setTextureRect(Rect(0, 0, _hpBarWidth, _hpBarHeight));
    hpBg->setAnchorPoint(Vec2(0.5f, 0.0f));
    hpBg->setPosition(_hpBar->getPosition());
    hpBg->setTag(100); // 新增：设置Tag，便于后续查找
    this->addChild(hpBg, 9);

    // 添加血条到士兵节点
    this->addChild(_hpBar, 10);

    // 新增：初始化时隐藏血条（包括背景）
    _hpBar->setVisible(false);
    hpBg->setVisible(false); // 同时隐藏血条背景
}

// 更新血条（仅需设置百分比，无需手动缩放）
void Soldier::updateHPBar() {
    if (!_hpBar) return;

    // 计算血量百分比（0~100）
    float hpPercent = std::max(0.0f, (_currentHP / _maxHP) * 100);
    _hpBar->setPercent(hpPercent);

}

Soldier* Soldier::create(Type type, Vec2 pos, float scale) {
    Soldier* soldier = nullptr;

    switch (type) {
        case Type::INFANTRY:
            soldier = Player1::create(pos, scale);
            break;
        case Type::ARCHER:
            soldier = Archer::create(pos, scale);
            break;
        case Type::CAVALRY:  // 新增骑兵
            soldier = Cavalry::create(pos, scale);
            break;
        case Type::MAGE:     // 新增法师
            soldier = Mage::create(pos, scale);
            break;
        default:
            CCLOG("未实现兵种类型：%d", (int)type);
            return nullptr;
    }
    if (!soldier) {
        CCLOG("兵种创建失败！类型：%d", (int)type);
    }
    return soldier;
}

bool Soldier::initBase(const std::string& idleImgPath, Vec2 pos, float scale) {
    if (!Sprite::initWithFile(idleImgPath)) {
        return false;
    }
    _idleImgPath = idleImgPath;
    this->setPosition(pos);
    this->setScale(scale);

    // ========== 新增：初始化属性 ==========
    initBaseAttributes();       // 加载1级基础属性
    updateAttributesByLevel();  // 根据当前等级计算最终属性
    _currentHP = _maxHP;        // 初始满血

    // ========== 新增：初始化_tilePos ==========
    // 这里需要将世界坐标转换为格子坐标
    // 但由于此时还没有地图节点，我们可以先设置为默认值
    // 稍后在moveToBuilding中会重新设置
    _tilePos = Vec2::ZERO;

    initHPBar();
    return true;
}

void Soldier::startTakeDamagePerSecond(float damagePerSec) {
    if (!isAlive() || _state == State::UNDER_ATTACK) return;

    _damagePerSec = damagePerSec;
    setState(State::UNDER_ATTACK);

    // 移除自动显示血条的代码，改为由攻击队列控制显示
    // setHPBarVisible(true);
    // if (_hpBar && _hpBar->getParent()) {
    //     auto hpBg = _hpBar->getParent()->getChildByTag(100);
    //     if (hpBg) hpBg->setVisible(true);
    // }

    // 定时器逻辑保持不变
    Director::getInstance()->getScheduler()->schedule(
        [this](float dt) {
            this->onTakeDamage(_damagePerSec);
        },
        this,
        1.0f,
        CC_REPEAT_FOREVER,
        0.0f,
        false,
        "DamageTimer_" + std::to_string((long long)this)
    );
}

// 2. 停止扣血（通过标识取消定时器）
void Soldier::stopTakeDamage() {
    Director::getInstance()->getScheduler()->unschedule(
        "DamageTimer_" + std::to_string((long long)this),
        this
    );
    if (isAlive()) {
        setState(State::WAITING);
        // 移除自动隐藏血条的代码，由攻击队列控制
        // setHPBarVisible(false);
        // if (_hpBar && _hpBar->getParent()) {
        //     auto hpBg = _hpBar->getParent()->getChildByTag(100);
        //     if (hpBg) hpBg->setVisible(false);
        // }
    }
}

// 修改 reduceHP 函数
void Soldier::reduceHP(float damage) {
    if (!isAlive()) return;

    _currentHP -= damage;
    if (_currentHP <= 0) {
        _currentHP = 0;
        _state = State::DEAD; // 1. 立即改变状态
        this->onDeath();       // 2. 触发死亡逻辑（如停止动画、销毁）
        return;
    }
    updateHPBar();
    // --- 新增：切换到受击状态 ---
    if (_currentHP > 0 && _state != State::DEAD) {
        this->setState(State::UNDER_ATTACK);
        this->setHPBarVisible(true); // 被打时确保血条显示
    }
    if (_currentHP <= 0) {
        setState(State::DEAD);
        onDeath(); // 添加死亡回调
    }
}

// 修改 onTakeDamage 函数
void Soldier::onTakeDamage(float damage) {
    if (!isAlive()) return;
    reduceHP(damage);
    if (_currentHP <= 0) {
        setState(State::DEAD);
        stopTakeDamage();
        onDeath(); // 确保死亡时调用
    }
}

// 确保 onDeath 函数正确实现
void Soldier::onDeath() {
    stopTakeDamage(); // 停止扣血
    if (_hpBar) _hpBar->setVisible(false);

    // 通知场景处理攻击队列
    if (_scene) {
        _scene->processAttackQueue();
    }

    // 销毁，确保所有逻辑执行完毕
    runAction(Sequence::create(
        CallFunc::create([this]() {
            this->removeFromParentAndCleanup(true);
            }),
        nullptr
    ));
}

void Soldier::setFlippedX(bool flipped) {
    //  翻转士兵自身（Sprite 类型，可直接调用 setFlippedX）
    this->Sprite::setFlippedX(flipped);

}

///////////////////////////////////////////////////
Vec2 Soldier::chooseTargetTile(Building* b)
{
    CCLOG("Soldier::chooseTargetTile");
    Vec2 start = _tilePos;
    auto tiles = b->getAttackTiles();
    CCLOG("pppppppppppppppp：(%.1f, %.1f)", start.x, start.y);
    CCLOG("oooooooooooooooooooooo：%d", (int)tiles.size());
    Vec2 best;
    int bestCost = INT_MAX;

    for (auto& t : tiles)
    {
        auto path = findPath(start, t);
        if (!path.empty() && path.size() < bestCost)
        {
            bestCost = path.size();
            best = t;
        }
    }
    return best;
}

void Soldier::moveToBuilding(Building* target) {
    if (!target) {
        CCLOG("错误：moveToBuilding传入的目标建筑为空");
        return;
    }

    CCLOG("Soldier::moveToBuilding 开始，目标建筑：%p", target);

    // 选择目标格子
    Vec2 targetTile = chooseTargetTile(target);
    CCLOG("当前士兵格子位置：(%.1f, %.1f)，目标格子：(%.1f, %.1f)",
        _tilePos.x, _tilePos.y, targetTile.x, targetTile.y);

    // 使用A*寻路
    auto path = findPath(_tilePos, targetTile);

    if (path.empty()) {
        CCLOG("警告：未找到路径到目标格子！");
        return;
    }

    CCLOG("找到路径，包含%d个点", (int)path.size());
    for (int i = 0; i < path.size(); i++) {
        CCLOG("  路径点[%d]: (%.1f,%.1f)", i, path[i].x, path[i].y);
    }

    Node* parentNode = this->getParent();
    if (!parentNode) {
        CCLOG("Soldier has no parent node");
        return;
        
    }
    
            // 尝试找到真正的地图 sprite（BattleScene1 中使用 CAMP_SPRITE_TAG 标记）
        Node * mapNode = parentNode->getChildByTag(1);
    if (!mapNode) {
                // 如果 parentNode 不是 Scene，退而求其次再从运行场景查找一次
            Scene * running = Director::getInstance()->getRunningScene();
        if (running) mapNode = running->getChildByTag(1);
        
    }
     if (!mapNode) {
        CCLOG("Cannot find map sprite (CAMP_SPRITE_TAG). Move aborted.");
        return;
        
    }

    Vector<FiniteTimeAction*> actions;

    // ========== 开始走路动画 ==========
    this->playWalkAnimation();
    CCLOG("开始播放走路动画");

    // 创建移动动作序列
    for (size_t i = 1; i < path.size(); i++) { // 从第1个点开始（第0个是起点）
        Vec2 tile = path[i];

  

               // 将格子坐标转换为地图局部坐标
        Vec2 mapLocal = tileToMapLocal(mapNode, tile.x, tile.y, false);

        // 从 tile 中心 → tile 底部
         mapLocal.y -= BATTLE_MAP_TILE_H * 0.5f;

        // 再根据士兵脚底微调（非常重要）
        mapLocal.y += 20.0f; 

        // 将地图局部坐标转换为世界坐标
        Vec2 worldPos = mapNode->convertToWorldSpace(mapLocal);
 

        // ========== 新增：根据移动方向设置翻转 ==========
        if (i == 1) {
            // 根据第一个移动方向决定是否翻转
            Vec2 prevTile = path[i - 1];
            if (tile.x < prevTile.x) {
                // 向左移动，需要翻转
                this->setFlippedX(true);
                CCLOG("士兵向左移动，设置翻转");
            }
            else if (tile.x > prevTile.x) {
                // 向右移动，不需要翻转
                this->setFlippedX(false);
                CCLOG("士兵向右移动，取消翻转");
            }
            // 上下移动不需要翻转
        }

        actions.pushBack(Sequence::create(
            MoveTo::create(0.3f, worldPos),
            CallFunc::create([this, tile]() {
                // 到达这个 tile 后，才更新逻辑位置
                _tilePos = tile;
                }),
            nullptr
        ));
    }

    // 在移动序列最后添加回调
    actions.pushBack(CallFunc::create([this]() {
        CCLOG("士兵移动动作序列完成");

        // ========== 新增：停止走路动画 ==========
        this->stopWalkAnimation();
        CCLOG("停止走路动画");

        // 检查是否设置了到达目标的回调
        if (this->onReachTarget) {
            this->onReachTarget(this);
        }
        else {
            CCLOG("警告：士兵没有设置onReachTarget回调");
        }
        }));

    // 执行动作序列
    runAction(Sequence::create(actions));
}
void Soldier::startAttacking(Building* target) {
    if (!target || _attackTarget == target) return;

    _attackTarget = target;
    // 开启定时器，每秒攻击一次
    this->schedule(CC_SCHEDULE_SELECTOR(Soldier::performAttack), 1.0f);
}

void Soldier::performAttack(float dt) {
    // 核心条件：只有活着、目标存在、且状态是等待/站定时才攻击
    if (this->isAlive() && _attackTarget && _attackTarget->isAlive()) {
        if (_state == State::WAITING || _state == State::UNDER_ATTACK) {

            _attackTarget->reduceHP(_attack);

            // 攻击动作视觉反馈：士兵跳一下
            auto jump = JumpBy::create(0.2f, Vec2::ZERO, 10, 1);
            this->runAction(jump);

        }
    }
    else {
        this->stopAttacking();
        this->onTargetDestroyed(); // 触发目标毁灭后的后续逻辑
    }
}

void Soldier::stopAttacking() {
    _attackTarget = nullptr;
    this->unschedule(CC_SCHEDULE_SELECTOR(Soldier::performAttack));
}
void Soldier::onTargetDestroyed() {
    _attackTarget = nullptr;

    if (this->isAlive()) {
        CCLOG("目标已摧毁，士兵重新寻找路径或继续前进");
        // 将状态切回 MOVING，以便触发后续的寻路或移动逻辑
        this->setState(State::MOVING);
    }
}