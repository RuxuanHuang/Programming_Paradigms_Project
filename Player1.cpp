#include "Player1.h"
#include "COCScene.h" // 包含场景头文件，访问队列

Player1* Player1::create(Vec2 pos, float scale) {
    auto p = new Player1();
    if (p && p->init(pos, scale)) {
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

bool Player1::init(Vec2 pos, float scale) {
 
    if (!initBase("player_walk1.png", pos, scale)) {
        return false;
    }

    _walkFrames = {
        "player_walk1.png",
        "player_walk2.png",
        "player_walk3.png",
        "player_walk4.png"
    };
    initWalkAnimation();
    return true;
}

// ========== 新增：1级基础属性 ==========
void Player1::initBaseAttributes() {
    _maxHP = 100;   // 1级最大生命值
    _attack = 10;   // 1级攻击力
}

// ========== 新增：等级成长公式（每级血+20，攻+3） ==========
void Player1::updateAttributesByLevel() {
    int level = getLevel();
    _maxHP = 100 + (level - 1) * 20; 
    _attack = 10 + (level - 1) * 3; 
}

void Player1::initWalkAnimation() {
    Vector<SpriteFrame*> frames;
    for (auto& path : _walkFrames) {
        auto texture = Director::getInstance()->getTextureCache()->addImage(path);
        auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0,
            texture->getContentSize().width, texture->getContentSize().height));
        frames.pushBack(frame);
    }
    _walkAnimation = Animation::createWithSpriteFrames(frames, 0.2f);
    _walkAnimation->setLoops(-1);
}

void Player1::playWalkAnimation() {
    if (_walkAction) stopAction(_walkAction);
    _walkAction = runAction(Animate::create(_walkAnimation));
}

void Player1::stopWalkAnimation() {
    if (_walkAction) {
        stopAction(_walkAction);
        _walkAction = nullptr;
        setTexture(_idleImgPath);
    }
}



void Player1::moveToTarget(Vec2 targetPos, float duration) {
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
        const float NEAR_DISTANCE = 500.0f;
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