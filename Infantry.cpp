#include "Infantry.h"

Infantry* Infantry::create(Vec2 pos, float scale) {
    auto infantry = new Infantry();
    if (infantry && infantry->init(pos, scale)) {
        infantry->autorelease();
        return infantry;
    }
    CC_SAFE_DELETE(infantry);
    return nullptr;
}

// 实现基类纯虚函数：初始化基础属性
void Infantry::initBaseAttributes() {
    // 步兵基础属性（示例值，可根据需求调整）
    _attack = 10;    // 基础攻击力
    _maxHP = 100;   // 基础生命值
}

// 实现基类纯虚函数：按等级更新属性
void Infantry::updateAttributesByLevel() {
    int level = getLevel();
    _maxHP = 100 + (level - 1) * 20;  // 1级100，2级120，3级140...
    _attack = 10 + (level - 1) * 3;   // 1级10，2级13，3级16...
}

bool Infantry::init(Vec2 pos, float scale) {
    // 步兵的idle图片和属性
    return initBase("infantry_idle.png", pos, scale);
}

void Infantry::initWalkAnimation() {
    // 步兵的行走帧（根据实际资源修改）
    _walkFrames = {
        "infantry_walk1.png",
        "infantry_walk2.png",
        "infantry_walk3.png",
        "infantry_walk4.png"
    };
    // 创建动画（复用原逻辑）
    Vector<SpriteFrame*> frames;
    for (const auto& path : _walkFrames) {
        auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(path);
        if (!frame) {
            auto texture = Director::getInstance()->getTextureCache()->addImage(path);
            frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
        }
        frames.pushBack(frame);
    }
    _walkAnimation = Animation::createWithSpriteFrames(frames, 0.2f);
    _walkAnimation->setLoops(-1);
}

void Infantry::playWalkAnimation() {
    if (!_walkAnimation) return;
    if (_walkAction) this->stopAction(_walkAction);
    _walkAction = this->runAction(Animate::create(_walkAnimation));
}

void Infantry::stopWalkAnimation() {
    if (_walkAction) {
        this->stopAction(_walkAction);
        _walkAction = nullptr;
        this->setTexture(_idleImgPath);
    }
}

void Infantry::moveToTarget(Vec2 targetPos, float duration) {
    // 计算当前位置与目标位置的X坐标差，判断是否需要翻转
    bool needFlip = this->getPositionX() > targetPos.x;
    setFlippedX(needFlip);
    auto moveAction = MoveTo::create(duration, targetPos);
    auto onFinish = CallFunc::create([this]() {
        if (onReachTarget) onReachTarget(this);
        });
    auto sequence = Sequence::create(moveAction, onFinish, nullptr);
    this->runAction(sequence);
    playWalkAnimation();
}