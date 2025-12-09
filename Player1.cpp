#include "Player1.h"

Player* Player::create(const std::string& imgPath, Vec2 pos, float scale)
{
    Player* player = new Player();
    if (player && player->init(imgPath, pos, scale))
    {
        player->autorelease();
        player->_idleImgPath = imgPath;
        return player;
    }
    CC_SAFE_DELETE(player);
    return nullptr;
}

bool Player::init(const std::string& imgPath, Vec2 pos, float scale)
{
    if (!Sprite::initWithFile(imgPath))
    {
        return false;
    }
    this->setPosition(pos);
    this->setScale(scale);
    return true;
}

void Player::initWalkAnimation(const std::vector<std::string>& walkFrames)
{
    if (walkFrames.size() != 4)
    {
        CCLOG("错误：走路动画必须传入4张帧图片！");
        return;
    }
    _walkFrames = walkFrames;

    Vector<SpriteFrame*> frames;
    for (const auto& path : walkFrames)
    {
        SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(path);
        if (!frame)
        {
            Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(path);
            frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
        }
        frames.pushBack(frame);
    }

    _walkAnimation = Animation::createWithSpriteFrames(frames, 0.2f);
    _walkAnimation->setLoops(-1);
}

void Player::playWalkAnimation()
{
    if (!_walkAnimation)
    {
        CCLOG("错误：未初始化走路动画！");
        return;
    }
    if (_walkAction)
    {
        this->stopAction(_walkAction);
        _walkAction = nullptr;
    }
    _walkAction = this->runAction(Animate::create(_walkAnimation));
}

void Player::stopWalkAnimation()
{
    if (_walkAction)
    {
        this->stopAction(_walkAction);
        _walkAction = nullptr;
        this->setTexture(_idleImgPath);
    }
}

void Player::moveToTarget(Vec2 targetPos, float duration)
{
    if (_moveAction)
    {
        this->stopAction(_moveAction);
        _moveAction = nullptr;
    }
    stopWalkAnimation();

    auto move = MoveTo::create(duration, targetPos);
    // 核心修改：移动到目标后 → 停止动画 → 延迟3秒 → 标记为待销毁
    auto moveEnd = CallFunc::create([this]() {
        this->stopWalkAnimation();
        this->_moveAction = nullptr;
        });
    // 延迟3秒（DelayTime::create(3.0f)） + 触发销毁回调
    auto delay = DelayTime::create(3.0f);
    auto waitToDestroy = CallFunc::create([this]() {
        // 发送自定义事件，通知HelloWorld销毁士兵（避免直接依赖）
        EventCustom event("soldier_need_destroy");
        event.setUserData(this);
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
        });
    // 动作序列：移动 → 停止动画 → 延迟3秒 → 触发销毁
    auto moveSeq = Sequence::create(move, moveEnd, delay, waitToDestroy, nullptr);

    _moveAction = this->runAction(moveSeq);
    playWalkAnimation();
}