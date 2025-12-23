#include "Mage.h"
#include "BattleScene1.h" // 包含场景头文件，访问队列

// ========== 新增：1级基础属性 ==========
void Mage::initBaseAttributes() {
    _maxHP = 1000;   // 1级最大生命值
    _attack = 10;   // 1级攻击力
}

// ========== 新增：等级成长公式（每级血+20，攻+3） ==========
void Mage::updateAttributesByLevel() {
    int level = getLevel();
    _maxHP = 1000 + (level - 1) * 20;  
    _attack = 10 + (level - 1) * 3;  
}

Mage* Mage::create(Vec2 pos, float scale) {
    auto p = new Mage();
    if (p && p->init(pos, scale)) {
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

bool Mage::init(Vec2 pos, float scale) {
    // 法师静止图片
    if (!initBase("mage_walk1.png", pos, scale)) {
        CCLOG("法师初始化失败：图片不存在！");
        return false;
    }
    // 法师行走帧图片
    _walkFrames = { "mage_walk1.png", "mage_walk2.png", "mage_walk3.png" };
    initWalkAnimation();
    return true;
}

void Mage::initWalkAnimation() {
    Vector<SpriteFrame*> frames;
    for (auto& path : _walkFrames) {
        auto texture = Director::getInstance()->getTextureCache()->addImage(path);
        auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0,
            texture->getContentSize().width, texture->getContentSize().height));
        frames.pushBack(frame);
    }
    // 法师动画最慢（0.25秒/帧）
    _walkAnimation = Animation::createWithSpriteFrames(frames, 0.25f);
    _walkAnimation->setLoops(-1);
}

void Mage::playWalkAnimation() {
    if (_walkAction) stopAction(_walkAction);
    _walkAction = runAction(Animate::create(_walkAnimation));
}

void Mage::stopWalkAnimation() {
    if (_walkAction) {
        stopAction(_walkAction);
        _walkAction = nullptr;
        setTexture(_idleImgPath);
    }
}

void Mage::moveToTarget(Vec2 targetPos, float duration) {
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
            // 加入攻击队列
            BattleScene1* scene = dynamic_cast<BattleScene1*>(this->getParent());
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