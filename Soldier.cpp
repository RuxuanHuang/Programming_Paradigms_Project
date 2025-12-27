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

Soldier::~Soldier() {
        _attackTarget = nullptr;
}

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
           
            return nullptr;
    }
    if (!soldier) {
       
    }
    return soldier;
}

// Soldier.cpp
bool Soldier::initBase(const std::string& idleImgPath, Vec2 pos, float scale) {
    if (!Sprite::initWithFile(idleImgPath)) {
        return false;
    }
    _idleImgPath = idleImgPath;
    this->setPosition(pos);
    this->setScale(scale);

    // ========== 初始化属性 ==========
    initBaseAttributes();       // 加载1级基础属性
    updateAttributesByLevel();  // 根据当前等级计算最终属性
    _currentHP = _maxHP;        // 初始满血

    // ========== 初始化攻击偏好 ==========
    initAttackPreference();     // 调用虚函数，子类可以重写

    // ========== 初始化_tilePos ==========
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

// 替换 Soldier::onDeath 为以下实现
// Soldier.cpp - 确保onDeath正确通知场景
void Soldier::onDeath() {
    // 立即设置状态为死亡
    _state = State::DEAD;
    _currentHP = 0;

    // 停止所有动作和定时器
    stopTakeDamage();
    stopAttacking();
    this->stopAllActions();
    this->unscheduleAllCallbacks();

    // 隐藏血条
    if (_hpBar) {
        _hpBar->setVisible(false);
        auto hpBg = this->getChildByTag(100);
        if (hpBg) hpBg->setVisible(false);
    }

    // 立即通知场景
    if (_scene) {
        // 确保从活跃列表移除
        _scene->removeSoldierFromList(this);
        // 通知场景处理队列
        _scene->processAttackQueue();

        // 延迟销毁视觉表现
        this->runAction(Sequence::create(
            FadeOut::create(0.3f),
            CallFunc::create([this]() {
                this->removeFromParent();
                }),
            nullptr
        ));
    }
}

void Soldier::setFlippedX(bool flipped) {
    //  翻转士兵自身（Sprite 类型，可直接调用 setFlippedX）
    this->Sprite::setFlippedX(flipped);

}

///////////////////////////////////////////////////
// Soldier.cpp - 修改 chooseTargetTile 为返回 bool + tile via reference
// Soldier.cpp - 修改 chooseTargetTile 函数，返回是否找到路径
bool Soldier::chooseTargetTile(Building* b, Vec2& outTile) {
    outTile = Vec2(0, 0);
    if (!b) return false;

    Vec2 start = _tilePos;
    auto tiles = b->getAttackTiles();
    int bestCost = INT_MAX;
    bool found = false;

    for (auto& t : tiles) {
        // 使用 findPath 查找路径
        auto path = findPath(start, t);
        if (!path.empty() && (int)path.size() < bestCost) {
            bestCost = (int)path.size();
            outTile = t;
            found = true;
        }
    }

    return found;
}

void Soldier::moveToBuilding(Building* target) {
    if (!target) {;
        return;
    }
    // 先检查士兵是否存活
    if (!this->isAlive()) {
        return;
    }

    
    // 选择目标格子
    Vec2 targetTile;
    if (!chooseTargetTile(target, targetTile)) {
        this->setState(State::WAITING);
        return;
    }
    auto mapSprite = dynamic_cast<Sprite*>(this->getParent()->getChildByTag(1));
    Vec2 currentRealTile = mapLocalToTile(mapSprite, mapSprite->convertToNodeSpace(this->getPosition()), false);
    
    this->setTilePosition(currentRealTile); // 更新 _tilePos
    // 新增调试日志：输出目标建筑的逻辑坐标
    CCLOG("DEBUG_TARGET_INFO: Target Name: %s, TilePos: (%.1f, %.1f)",
        target->getBuildingName().c_str(), target->getTileX(), target->getTileY());
    Vec2 start = _tilePos;
    auto path = findPath(_tilePos, targetTile);
    if (path.empty()) {
        this->setState(State::WAITING);
        return;
    }


    for (int i = 0; i < path.size(); i++) {
       
    }

    Node* parentNode = this->getParent();
    if (!parentNode) {
       
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
        return;
        
    }

    Vector<FiniteTimeAction*> actions;

    // ========== 开始走路动画 ==========
    this->playWalkAnimation();

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
            }
            else if (tile.x > prevTile.x) {
                // 向右移动，不需要翻转
                this->setFlippedX(false);
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

    // 改为：
    actions.pushBack(CallFunc::create([this]() {
        // 停止走路动画
        this->stopWalkAnimation();

        // 设置状态为等待（到达目标了）
        this->setState(State::WAITING);

        // 调用虚函数（允许子类重写行为）
        this->onReachTargetCallback();
        }));
    // 执行动作序列
    runAction(Sequence::create(actions));
}
// Soldier.cpp - 在 startAttacking 函数中添加定时检查
// --- 替换或修改这两个函数 ---

// Soldier.cpp - 替换 startAttacking / stopAttacking 为下面内容

void Soldier::startAttacking(Building* target) {
    if (!target) return;

    // 只设置目标指针（不 retain）
    _attackTarget = target;;

    // 确保 performAttack 已注册
    this->unschedule(CC_SCHEDULE_SELECTOR(Soldier::performAttack));
    this->schedule(CC_SCHEDULE_SELECTOR(Soldier::performAttack), 1.0f);

    // 目标检测定时器（Director Scheduler）
    Director::getInstance()->getScheduler()->unschedule("TargetCheckTimer", this);
    Director::getInstance()->getScheduler()->schedule(
        [this](float dt) {
            if (!_attackTarget || !_attackTarget->getParent() || !_attackTarget->isAlive()) {
                this->stopAttacking();
                this->onTargetDestroyed();
            }
        },
        this,
        0.5f,
        false,
        "TargetCheckTimer"
    );

    this->setHPBarVisible(true);
}

void Soldier::stopAttacking() {
    // 取消定时器
    this->unschedule(CC_SCHEDULE_SELECTOR(Soldier::performAttack));
    Director::getInstance()->getScheduler()->unschedule("TargetCheckTimer", this);

    // 不再 release 目标（由场景/父节点管理生命周期），仅清空指针
    if (_attackTarget) {
        
        _attackTarget = nullptr;
    }
    
}

// Replace the beginning of Soldier::performAttack with safer checks
// Soldier.cpp - 替换 performAttack 实现（安全版）
void Soldier::performAttack(float dt) {
    // 先拿到当前场景
    auto scene = dynamic_cast<BattleScene1*>(Director::getInstance()->getRunningScene());
    if (!scene) {
        // 没有场景则退回到安全状态
        this->stopAttacking();
        return;
    }

    // 如果没有目标，直接尝试切换目标
    if (!_attackTarget) {
       
        this->stopAttacking();
        this->onTargetDestroyed();
        return;
    }

    // 安全调用场景接口来对目标造成伤害（场景会先验证目标是否仍有效）
    bool didDamage = scene->applyDamageToBuilding(_attackTarget, _attack);

    // applyDamageToBuilding 返回 false 表示目标在场景中不存在或已无效
    if (!didDamage) {
        this->stopAttacking();
        this->onTargetDestroyed();
        return;
    }

    // 成功造成伤害后，可以播放攻击动作（跳跃、攻击动画等）
    if (_state == State::WAITING || _state == State::UNDER_ATTACK) {
        auto jump = JumpBy::create(0.2f, Vec2::ZERO, 10, 1);
        this->runAction(jump);
    }
}

// Soldier.cpp - 实现 canReachBuilding 函数
bool Soldier::canReachBuilding(Building* building) {
    if (!building) return false;

    Vec2 targetTile;
    return chooseTargetTile(building, targetTile);
}

void Soldier::onTargetDestroyed() {
    // 1. 停止当前的攻击行为
    this->stopAttacking();

    if (!this->isAlive()) {
        return;
    }

    // 2. 清除当前攻击目标
    _attackTarget = nullptr;

    // 3. 如果绑定到场景，让场景帮助寻找下一个目标建筑
    if (_scene) {
        // 将状态设置为移动中
        this->setState(State::MOVING);

        // 使用场景的帮助函数寻找下一个目标
        // 这个函数会考虑偏好和路径可达性
        Building* nextTarget = _scene->getNextTargetBuilding(this);

        if (nextTarget && nextTarget->isAlive()) {
            // 设置新的攻击目标
            _attackTarget = nextTarget;

            // 立即开始移动到新建筑
            this->moveToBuilding(_attackTarget);

            // 设置到达目标时的回调
            this->onReachTarget = [this](Soldier* s) {
                // 确保目标仍然存在且存活
                if (s->getAttackTarget() && s->getAttackTarget()->isAlive()) {
                    s->setState(Soldier::State::WAITING);
                    s->startAttacking(s->getAttackTarget());
                }
                else {
                    // 如果目标已经没了，继续寻找下一个
                    s->onTargetDestroyed();
                }
                };
        }
        else {
            // 没有找到任何目标
            this->setState(State::WAITING);
        }
    }
    else {
        this->setState(State::WAITING);
    }
}

// ========== 新增：寻找最近偏好建筑 ==========
// ========== 新增：寻找最近偏好建筑 ==========
Building* Soldier::findNearestPreferredBuilding() {
    if (!_scene) return nullptr;

    // 将_scene转换为BattleScene1指针
    BattleScene1* battleScene = dynamic_cast<BattleScene1*>(_scene);
    if (!battleScene) return nullptr;

    // 如果没有任何偏好，直接返回null
    if (_attackPreference == BuildingPreference::ANY || _preferredBuildingTypes.empty()) {
        return nullptr;
    }

    // 调用BattleScene1的帮助函数
    return battleScene->findNearestPreferredBuildingForSoldier(this, _attackPreference, _preferredBuildingTypes);
}

Building* Soldier::findNearestBuilding() {
    if (!_scene) return nullptr;

    // 将_scene转换为BattleScene1指针
    BattleScene1* battleScene = dynamic_cast<BattleScene1*>(_scene);
    if (!battleScene) return nullptr;

    // 调用BattleScene1的帮助函数
    return battleScene->findNearestBuildingForSoldier(this);
}