#include "COCScene.h"
#include "Player1.h"
#include <ctime> 
#include <vector>
#include <algorithm> // 用于查找士兵列表

USING_NS_CC;

Scene* COC::createScene()
{
    return COC::create();
}

static void problemLoading(const char* filename)
{
    printf("加载文件时出错: %s\n", filename);
    printf("根据编译方式的不同，你可能需要在 COCScene.cpp 中的文件名前添加 'Resources/' 路径\n");
}

bool COC::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    srand((unsigned)time(NULL));

    // 固定进攻目标 + 统一速度
    Vec2 attackTargetPos = Vec2(origin.x + visibleSize.width - 50, origin.y + visibleSize.height / 2);
    const float SOLDIER_SPEED = 200.0f;

    // 背景
    auto sprite = Sprite::create("backgroud.png");
    if (sprite == nullptr)
    {
        problemLoading("'backgroud.png'");
    }
    else
    {
        sprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        sprite->setScale(visibleSize.width / sprite->getContentSize().width, visibleSize.height / sprite->getContentSize().height);
        this->addChild(sprite, 0);
    }

    // ========== 士兵数量标签 ==========
    auto soldierCountLabel = Label::createWithSystemFont("0/10", "Arial", 48);
    soldierCountLabel->setColor(Color3B::RED);
    soldierCountLabel->enableBold();
    soldierCountLabel->enableOutline(Color4B::BLACK, 3);
    soldierCountLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    soldierCountLabel->setLocalZOrder(1);
    soldierCountLabel->setTag(1001);
    this->addChild(soldierCountLabel);
    CCLOG("标签最终位置：x=%.2f, y=%.2f", soldierCountLabel->getPositionX(), soldierCountLabel->getPositionY());

    // ========== 销毁事件监听 ==========
    auto destroyListener = EventListenerCustom::create("soldier_need_destroy", [](EventCustom* event) {
        Player* soldier = (Player*)event->getUserData();
        if (!soldier) return;
        // 仅销毁士兵节点，不处理数量
        soldier->removeFromParent();
        });
    _eventDispatcher->addEventListenerWithFixedPriority(destroyListener, 1);

    // ========== 触摸监听 ==========
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);

    touchListener->onTouchBegan = [this, attackTargetPos, SOLDIER_SPEED](Touch* touch, Event* event) -> bool {
        // 数量上限判断
        if (this->_currentSoldierCount >= this->MAX_SOLDIER_COUNT)
        {
            CCLOG("士兵数量已达上限（10），无法生成！");
            return true;
        }

        // 出生位置（带偏移）
        Vec2 bornPos = touch->getLocation();
        float randomOffsetX = (rand() % 20) - 10;
        float randomOffsetY = (rand() % 20) - 10;
        bornPos.x += randomOffsetX;
        bornPos.y += randomOffsetY;

        // 计算移动耗时
        float distance = bornPos.distance(attackTargetPos);
        float duration = distance / SOLDIER_SPEED;
        if (duration < 0.1f) duration = 0.1f;

        // 创建士兵
        auto soldier = Player::create("player_walk1.png", bornPos, 2.0f);
        if (soldier != nullptr)
        {
            this->addChild(soldier, 1);

            // 初始化走路动画
            std::vector<std::string> walkFrames = {
                "player_walk1.png",
                "player_walk2.png",
                "player_walk3.png",
                "player_walk4.png"
            };
            soldier->initWalkAnimation(walkFrames);

            // 移动到目标
            soldier->moveToTarget(attackTargetPos, duration);

            // 更新累计释放数量
            this->_currentSoldierCount++;
            CCLOG("生成士兵，累计释放：%d/10", this->_currentSoldierCount);

            // 更新标签
            auto label = (Label*)this->getChildByTag(1001);
            if (label)
            {
                label->setString(StringUtils::format("%d/10", this->_currentSoldierCount));
            }
        }

        return true;
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    // 目标标记（调试）
    auto targetMarker = DrawNode::create();
    targetMarker->drawCircle(attackTargetPos, 10, 0, 30, false, Color4F::RED);
    this->addChild(targetMarker, 2);

    // 敌方单位
    auto player2 = Sprite::create("cat.png");
    if (player2 == nullptr)
    {
        problemLoading("'cat.png'");
    }
    else
    {
        player2->setPosition(attackTargetPos);
        player2->setScale(2.0f);
        this->addChild(player2, 1);

        auto player2_rotate = RotateBy::create(1.0f, -360);
        auto player2_repeat = RepeatForever::create(player2_rotate);
        player2->runAction(player2_repeat);
    }

    return true;
}
void COC::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}