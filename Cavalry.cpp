#include "Cavalry.h"
#include "COCScene.h" // 包含场景头文件，访问队列

// ========== 新增：1级基础属性 ==========
void Cavalry::initBaseAttributes() {
    _maxHP = 50;   // 1级最大生命值
    _attack = 10;   // 1级攻击力
}

// ========== 新增：等级成长公式（每级血+20，攻+3） ==========
void Cavalry::updateAttributesByLevel() {
    int level = getLevel();
    _maxHP = 50 + (level - 1) * 20; 
    _attack = 10 + (level - 1) * 3;   
}

Cavalry* Cavalry::create(Vec2 pos, float scale) {
    auto p = new Cavalry();
    if (p && p->init(pos, scale)) {
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

bool Cavalry::init(Vec2 pos, float scale) {
    // 骑兵静止图片
    if (!initBase("cavalry_walk1.png", pos, scale)) {
        CCLOG("骑兵初始化失败：图片不存在！");
        return false;
    }
    // 骑兵行走帧图片
    _walkFrames = { "cavalry_walk1.png", "cavalry_walk2.png", "cavalry_walk3.png", "cavalry_walk4.png" };
    initWalkAnimation();
    return true;
}

void Cavalry::initWalkAnimation() {
    Vector<SpriteFrame*> frames;
    for (auto& path : _walkFrames) {
        auto texture = Director::getInstance()->getTextureCache()->addImage(path);
        auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0,
            texture->getContentSize().width, texture->getContentSize().height));
        frames.pushBack(frame);
    }
    // 骑兵动画更快（0.1秒/帧）
    _walkAnimation = Animation::createWithSpriteFrames(frames, 0.1f);
    _walkAnimation->setLoops(-1);
}

void Cavalry::playWalkAnimation() {
    if (_walkAction) stopAction(_walkAction);
    _walkAction = runAction(Animate::create(_walkAnimation));
}

void Cavalry::stopWalkAnimation() {
    if (_walkAction) {
        stopAction(_walkAction);
        _walkAction = nullptr;
        setTexture(_idleImgPath);
    }
}

void Cavalry::moveToTarget(Vec2 targetPos, float duration) {
    if (_moveAction) stopAction(_moveAction);

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