
#include "Archer.h"
#include "COCScene.h" // 包含场景头文件，访问队列

// ========== 新增：1级基础属性（步兵：血厚、攻中等） ==========
void Archer::initBaseAttributes() {
    _maxHP = 50;   // 1级最大生命值
    _attack = 100;   // 1级攻击力
}

// ========== 新增：等级成长公式（每级血+20，攻+3） ==========
void Archer::updateAttributesByLevel() {
    int level = getLevel();
    _maxHP = 50 + (level - 1) * 20;  // 1级100，2级120，3级140...
    _attack = 100 + (level - 1) * 3;   // 1级10，2级13，3级16...
}

Archer* Archer::create(Vec2 pos, float scale) {
    auto p = new Archer();
    if (p && p->init(pos, scale)) {
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

bool Archer::init(Vec2 pos, float scale) {
    // 替换为弓箭手的静止图片
    if (!initBase("archer_walk1.png", pos, scale)) {
        return false;
    }
    // 替换为弓箭手的行走帧图片
    _walkFrames = { "archer_walk1.png", "archer_walk2.png", "archer_walk3.png" };
    initWalkAnimation();
    return true;
}

// 以下函数实现与Player1类似，可根据弓箭手特性调整（如移动速度、动画帧间隔）
void Archer::initWalkAnimation() {
    Vector<SpriteFrame*> frames;
    for (auto& path : _walkFrames) {
        auto texture = Director::getInstance()->getTextureCache()->addImage(path);
        auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0,
            texture->getContentSize().width, texture->getContentSize().height));
        frames.pushBack(frame);
    }
    _walkAnimation = Animation::createWithSpriteFrames(frames, 0.15f); // 弓箭手动画更快
    _walkAnimation->setLoops(-1);
}

void Archer::playWalkAnimation() {
    if (_walkAction) stopAction(_walkAction);
    _walkAction = runAction(Animate::create(_walkAnimation));
}

void Archer::stopWalkAnimation() {
    if (_walkAction) {
        stopAction(_walkAction);
        _walkAction = nullptr;
        setTexture(_idleImgPath);
    }
}

// Archer.cpp 中的 moveToTarget 函数
void Archer::moveToTarget(Vec2 targetPos, float duration) {
    if (_moveAction) stopAction(_moveAction);
    // 计算当前位置与目标位置的X坐标差，判断是否需要翻转
    bool needFlip = this->getPositionX() > targetPos.x;
    setFlippedX(needFlip);

    bool* hasEntered = new bool(false);
    auto move = MoveTo::create(duration, targetPos);

    // 范围检测：进入攻击范围则加入队列
    auto checkDistance = CallFunc::create([this, targetPos, hasEntered]() {
        if (*hasEntered) return;

        float distance = this->getPosition().distance(targetPos);
        const float NEAR_DISTANCE = 50.0f;
        if (distance <= NEAR_DISTANCE) {
            *hasEntered = true;
            // 加入攻击队列（替代原有销毁队列）
            COC* scene = dynamic_cast<COC*>(this->getParent());
            if (scene) {
                scene->addToAttackQueue(this);
            }
        }
        });

    auto checkRepeat = Repeat::create(Sequence::create(checkDistance, DelayTime::create(0.1f), nullptr), 1000);
    _moveAction = runAction(Spawn::create(move, checkRepeat, nullptr));
    playWalkAnimation();

    auto cleanup = CallFunc::create([hasEntered]() {
        delete hasEntered;
        });
    runAction(Sequence::create(static_cast<FiniteTimeAction*>(_moveAction), cleanup, nullptr));
}