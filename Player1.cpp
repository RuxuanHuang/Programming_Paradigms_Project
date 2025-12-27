#include "Player1.h"
#include "BattleScene1.h" // 包含场景头文件，访问队列

Player1::~Player1() {
    if (_walkAnimation) {
        _walkAnimation->release();
        _walkAnimation = nullptr;
    }
}

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
    // 创建 Animation 并 retain 它，保证跨帧存在
    _walkAnimation = Animation::createWithSpriteFrames(frames, 0.2f);
    if (_walkAnimation) {
        _walkAnimation->retain();
        _walkAnimation->setLoops(-1);
    }
}

void Player1::playWalkAnimation() {
    // 保护性检查
    if (!_walkAnimation) {
        CCLOG("Warning: playWalkAnimation called but _walkAnimation == nullptr");
        return;
    }

    // 如果之前有正在运行的 walk 动作，先停止并置空（避免悬垂）
    if (_walkAction) {
        stopAction(_walkAction);
        _walkAction = nullptr;
    }

    // 创建 Animate 并运行，保存返回的 Action*
    auto animAction = Animate::create(_walkAnimation);
    if (animAction) {
        _walkAction = this->runAction(animAction);
    }
    else {
        CCLOG("Warning: Animate::create returned nullptr");
    }
}

void Player1::stopWalkAnimation() {
    if (_walkAction) {
        stopAction(_walkAction);
        _walkAction = nullptr;
    }
    // 回到 idle 贴图
    setTexture(_idleImgPath);
}



void Player1::moveToTarget(Vec2 targetPos, float duration) {
    if (_moveAction) stopAction(_moveAction);

    // 方向处理
    bool needFlip = this->getPositionX() > targetPos.x;
    setFlippedX(needFlip);

    bool* hasEntered = new bool(false);
    auto move = MoveTo::create(duration, targetPos);

    auto checkDistance = CallFunc::create([this, targetPos, hasEntered]() {
        if (*hasEntered) return;
        float distance = this->getPosition().distance(targetPos);
        const float NEAR_DISTANCE = 500.0f;
        if (distance <= NEAR_DISTANCE) {
            *hasEntered = true;
            BattleScene1* scene = dynamic_cast<BattleScene1*>(this->getParent());
            if (scene) {
                scene->addToAttackQueue(this);
            }
        }
        });

    // 使用 RepeatForever 或合理次数的 Repeat，避免把 runAction 的返回值再作为动作项使用
    auto checkSeq = Sequence::create(checkDistance, DelayTime::create(0.1f), nullptr);
    auto checkRepeat = RepeatForever::create(checkSeq);

    // 先创建 Spawn（或 Sequence），不要把 runAction(...) 的返回值 cast 回去再作为动作使用
    auto spawn = Spawn::create(move, checkRepeat, nullptr);

    // cleanup lambda
    auto cleanup = CallFunc::create([hasEntered]() {
        delete hasEntered;
        });

    // 把 spawn 放入 sequence，runAction 返回值保存在 _moveAction 中
    auto seq = Sequence::create(spawn, cleanup, nullptr);
    _moveAction = this->runAction(seq);

    // 播放走路动画
    playWalkAnimation();
}