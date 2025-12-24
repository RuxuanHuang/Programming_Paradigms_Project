#include "Store.h"

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
            this->onCardClicked(data.cardName);
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, cardIcon);

    this->addChild(cardIcon);
    cardSprites.pushBack(cardIcon);
}

void Store::onCardClicked(const std::string& cardName) {
    // 播放购买音效（暂时用点击音效代替）
    AudioEngine::play2d("click.mp3", false, 0.8f);

    // 如果有设置回调，调用回调
    if (_cardSelectCallback) {
        _cardSelectCallback(cardName);
    }

    // 返回大本营场景
    Director::getInstance()->popScene();
}