#include "Store.h"
#include"resources.h"

bool Store::init() {
    if (!Scene::init()) {
        return false;
    }

    // 获取可见区域大小
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建背景
    auto background = Sprite::create("shop.png");
    if (background != nullptr) {
        background->setScaleX(visibleSize.width / background->getContentSize().width);
        background->setScaleY(visibleSize.height / background->getContentSize().height);
        background->setPosition(Vec2(visibleSize.width / 2 + origin.x,
            visibleSize.height / 2 + origin.y));
        this->addChild(background, -1);
    }
    else {
        auto fallbackBackground = LayerColor::create(Color4B(50, 50, 80, 255),
            visibleSize.width, visibleSize.height);
        this->addChild(fallbackBackground, -1);
    }

    // 初始化卡片数据
    initCardData();
    createUI();

    createResourceDisplay();
    return true;
}

void Store::initCardData() {
    // 初始化8种建筑卡片数据

    cardDataList = {
        {"Cannon", "StoreCards/CannonCard.png", 250,"Gold" ,false},
        {"Archer Tower", "StoreCards/ArcherTowerCard.png", 1000,"Gold" ,false},
        {"Wall", "StoreCards/WallCard.png", 0, "Gold",false},
        {"Army Camp", "StoreCards/ArmyCampCard.png", 200,"Elixir", false},
        {"Gold Mine", "StoreCards/GoldMineCard.png", 150,"Elixir", false},
        {"Elixir Collector", "StoreCards/ElixirCollectorCard.png", 150,"Gold", false},
        {"Gold Storage", "StoreCards/GoldStorageCard.png", 300, "Elixir",false},
        {"Elixir Storage", "StoreCards/ElixirStorageCard.png", 300,"Elixir", false}
    };
}

void Store::createResourceDisplay() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 中间偏左下的位置（屏幕左下角往上190，往右300的位置）
    Vec2 basePosition = Vec2(origin.x + 300, origin.y + 190);

    // 获取资源管理器实例
    auto resourceManager = ResourceManager::getInstance();


    goldLabel = Label::createWithTTF("0", "fonts/arial.ttf", 28);
    goldLabel->setPosition(Vec2(basePosition.x + 30, basePosition.y));
    goldLabel->setTextColor(Color4B::YELLOW);
    this->addChild(goldLabel);


    elixirLabel = Label::createWithTTF("0", "fonts/arial.ttf", 28);
    elixirLabel->setPosition(Vec2(basePosition.x + 300, basePosition.y));
    elixirLabel->setTextColor(Color4B::MAGENTA);
    this->addChild(elixirLabel);

    // 初始更新资源显示
    updateResourceDisplay();


}


void Store::updateResourceDisplay() {
    auto resourceManager = ResourceManager::getInstance();

    // 更新金币显示
    int goldAmount = resourceManager->getGoldAmount();
    goldLabel->setString(std::to_string(goldAmount));

    // 更新圣水显示
    int elixirAmount = resourceManager->getElixirAmount();
    elixirLabel->setString(std::to_string(elixirAmount));
}



void Store::createUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();


    // 创建返回按钮
    auto backButton = ui::Button::create("back.png", "", "");

    backButton->setPosition(Vec2(visibleSize.width - 60, visibleSize.height - 60));
    backButton->setScale(0.6f);
    backButton->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            AudioEngine::play2d("click.mp3", false, 0.8f);
            Director::getInstance()->popScene();
        }
        });
    this->addChild(backButton, 1);

    // 创建卡片网格布局
    int cardsPerRow = 4;
    float cardWidth = 280;
    float cardHeight = 240;
    float startX = (visibleSize.width - cardsPerRow * cardWidth) / 2 + cardWidth / 2;
    float startY = visibleSize.height - 400;

    for (int i = 0; i < cardDataList.size(); i++) {
        int row = i / cardsPerRow;
        int col = i % cardsPerRow;

        Vec2 position = Vec2(
            startX + col * cardWidth,
            startY - row * (cardHeight + 20)
        );

        createCard(cardDataList[i], position);
    }
}



void Store::createCard(const StoreCardData& data, Vec2 position) {
    // 创建卡片图标
    auto cardIcon = Sprite::create(data.iconPath);
    cardIcon->setPosition(Vec2(position.x, position.y + 50));
    cardIcon->setScale(0.48f);

    // 添加触摸事件监听器
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = [cardIcon, this, data](Touch* touch, Event* event) -> bool {
        if (cardIcon->getBoundingBox().containsPoint(touch->getLocation())) {
            // 触摸开始时缩小一点，提供反馈
            cardIcon->runAction(ScaleTo::create(0.1f, 0.45f));
            return true;
        }
        return false;
        };

    touchListener->onTouchEnded = [cardIcon, this, data](Touch* touch, Event* event) {
        // 恢复大小
        cardIcon->runAction(ScaleTo::create(0.1f, 0.48f));

        // 检查是否在卡片区域内释放
        if (cardIcon->getBoundingBox().containsPoint(touch->getLocation())) {
            AudioEngine::play2d("click.mp3", false, 0.8f);
            this->onCardClicked(data);
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, cardIcon);

    this->addChild(cardIcon);
    cardSprites.pushBack(cardIcon);
}

void Store::onCardClicked(const StoreCardData& data) {
    // 播放购买音效
    AudioEngine::play2d("click.mp3", false, 0.8f);

    // 获取资源管理器
    auto resourceManager = ResourceManager::getInstance();
    bool purchaseSuccess = false;

    // 根据货币类型进行购买判断
    if (data.costType == "Gold") {
        // 检查金币是否足够
        if (resourceManager->canAffordGold(data.price)) {
            // 尝试购买
            purchaseSuccess = resourceManager->makeGoldPurchase(data.price);
            if (purchaseSuccess) {
                CCLOG("Purchased %s for %d gold", data.cardName.c_str(), data.price);
                // 显示购买成功提示
                showPurchaseMessage("Purchase Successful!", Color4B::GREEN);
            }
            else {
                CCLOG("Failed to purchase %s", data.cardName.c_str());
                showPurchaseMessage("Purchase Failed!", Color4B::RED);
            }
        }
        else {
            // 金币不足
            showPurchaseMessage("Not Enough Gold!", Color4B::RED);
            CCLOG("Not enough gold to purchase %s. Need: %d, Have: %d",
                data.cardName.c_str(), data.price, resourceManager->getGoldAmount());
            return; // 资源不足，不继续执行
        }
    }
    else if (data.costType == "Elixir") {
        // 检查圣水是否足够
        if (resourceManager->canAffordElixir(data.price)) {
            // 尝试购买
            purchaseSuccess = resourceManager->makeElixirPurchase(data.price);
            if (purchaseSuccess) {
                CCLOG("Purchased %s for %d elixir", data.cardName.c_str(), data.price);
                showPurchaseMessage("Purchase Successful!", Color4B::GREEN);
            }
            else {
                CCLOG("Failed to purchase %s", data.cardName.c_str());
                showPurchaseMessage("Purchase Failed!", Color4B::RED);
            }
        }
        else {
            // 圣水不足
            showPurchaseMessage("Not Enough Elixir!", Color4B::RED);
            CCLOG("Not enough elixir to purchase %s. Need: %d, Have: %d",
                data.cardName.c_str(), data.price, resourceManager->getElixirAmount());
            return; // 资源不足，不继续执行
        }
    }
    else if (data.price == 0) {
        // 免费物品
        purchaseSuccess = true;
        CCLOG("Acquired free item: %s", data.cardName.c_str());
        showPurchaseMessage("Item Acquired!", Color4B::GREEN);
    }
    else {
        // 未知货币类型
        showPurchaseMessage("Unknown currency type!", Color4B::RED);
        CCLOG("Unknown cost type: %s for card: %s", data.costType.c_str(), data.cardName.c_str());
        return;
    }

    // 如果购买成功
    if (purchaseSuccess) {
        // 更新资源显示
        updateResourceDisplay();

        // 如果有设置回调，调用回调
        if (_cardSelectCallback) {
            _cardSelectCallback(data.cardName);
        }

        // 等待一下让玩家看到购买成功消息，然后返回
        this->runAction(Sequence::create(
            DelayTime::create(0.8f),
            CallFunc::create([this]() {
                Director::getInstance()->popScene();
                }),
            nullptr
        ));
    }
    else {
        // 购买失败，不返回场景，让玩家继续选择
        return;
    }
}

void Store::showPurchaseMessage(const std::string& message, Color4B color) {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建消息标签
    auto messageLabel = Label::createWithTTF(message, "fonts/arial.ttf", 30);
    messageLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 100));
    messageLabel->setTextColor(color);
    messageLabel->setOpacity(0);
    this->addChild(messageLabel, 10);

    // 淡入淡出动画
    auto fadeIn = FadeIn::create(0.3f);
    auto delay = DelayTime::create(1.0f);
    auto fadeOut = FadeOut::create(0.5f);
    auto remove = RemoveSelf::create();

    messageLabel->runAction(Sequence::create(fadeIn, delay, fadeOut, remove, nullptr));
}
