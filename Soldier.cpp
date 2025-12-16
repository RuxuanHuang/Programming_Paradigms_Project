#include "Soldier.h"
#include "COCScene.h"
#include "Player1.h"    // 步兵
#include "Archer.h"     // 弓箭手
#include "Cavalry.h"    // 骑兵
#include "Mage.h"       // 法师
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

    initHPBar();
    return true;
}

// 1. 开始每秒扣血（无 Timer 版本）
void Soldier::startTakeDamagePerSecond(float damagePerSec) {
    if (!isAlive() || _state == State::UNDER_ATTACK) return;

    _damagePerSec = damagePerSec;
    setState(State::UNDER_ATTACK);

    // 新增：显示血条（包括背景）
    setHPBarVisible(true);
    if (_hpBar && _hpBar->getParent()) {
        // 找到血条背景并显示（假设背景是血条的同级节点，且在血条下方）
        auto hpBg = _hpBar->getParent()->getChildByTag(100); // 后续步骤会给背景设置Tag
        if (hpBg) hpBg->setVisible(true);
    }

    // 核心：用 Scheduler 直接调度，无需 Timer 指针
    Director::getInstance()->getScheduler()->schedule(
        [this](float dt) { // 每秒执行的回调
            this->onTakeDamage(_damagePerSec);
        },
        this,               // 绑定当前士兵节点
        1.0f,               // 间隔1秒
        CC_REPEAT_FOREVER,  // 无限重复
        0.0f,               // 延迟0秒开始
        false,              // 不暂停
        "DamageTimer_" + std::to_string((long long)this) // 唯一定时器标识（避免冲突）
    );
}

// 2. 停止扣血（通过标识取消定时器）
void Soldier::stopTakeDamage() {
    // 取消当前士兵的扣血定时器
    Director::getInstance()->getScheduler()->unschedule(
        "DamageTimer_" + std::to_string((long long)this),
        this
    );
    if (isAlive()) {
        setState(State::WAITING);
        // 新增：隐藏血条（包括背景）
        setHPBarVisible(false);
        if (_hpBar && _hpBar->getParent()) {
            auto hpBg = _hpBar->getParent()->getChildByTag(100);
            if (hpBg) hpBg->setVisible(false);
        }
    }
}

// 修改 reduceHP 函数
void Soldier::reduceHP(float damage) {
    if (!isAlive()) return;
    _currentHP = std::max(0.0f, _currentHP - damage);
    updateHPBar();
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
