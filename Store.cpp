#include"Store.h"


bool Store::init()
{
    if (!Layer::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建背景图片
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto background = Sprite::create("shop.png");
    if (background != nullptr)
    {
        background->setScaleX(visibleSize.width / background->getContentSize().width);
        background->setScaleY(visibleSize.height / background->getContentSize().height);
        background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        this->addChild(background, -1);
    }
    else
    {
        auto fallbackBackground = LayerColor::create(Color4B(0, 150, 255, 255),
            visibleSize.width, visibleSize.height);
        this->addChild(fallbackBackground, -1);
    }

    // 在顶部显示玩家资源
    displayPlayerResources();

    // 添加返回按钮
    auto backButton = Sprite::create("back_button.png");
    if (backButton == nullptr) {
        backButton = Sprite::create();
        backButton->setTextureRect(Rect(0, 0, 60, 60));
        backButton->setColor(Color3B::RED);

        auto drawNode = DrawNode::create();
        drawNode->drawLine(Vec2(10, 10), Vec2(50, 50), Color4F::WHITE);
        drawNode->drawLine(Vec2(50, 10), Vec2(10, 50), Color4F::WHITE);
        backButton->addChild(drawNode);
    }

    backButton->setPosition(Vec2(40, visibleSize.height - 100));
    backButton->setScale(0.8f);

    // 返回按钮触摸事件
    auto backListener = EventListenerTouchOneByOne::create();
    backListener->setSwallowTouches(true);
    backListener->onTouchBegan = [backButton](Touch* touch, Event* event) {
        if (backButton->getBoundingBox().containsPoint(touch->getLocation())) {
            backButton->runAction(ScaleTo::create(0.1, 0.9));
            return true;
        }
        return false;
        };

    backListener->onTouchEnded = [this, backButton](Touch* touch, Event* event) {
        backButton->runAction(ScaleTo::create(0.1, 0.8));
        if (backButton->getBoundingBox().containsPoint(touch->getLocation())) {
            // 播放点击音效
            AudioEngine::play2d("click.mp3", false, 0.8f);

            this->setVisible(false);
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(backListener, backButton);
    this->addChild(backButton, 10);

    // 添加说明文字
    auto instructionLabel = Label::createWithTTF("点击图片购买并放置到场景中", "fonts/arial.ttf", 24);
    instructionLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 150));
    instructionLabel->setColor(Color3B::YELLOW);
    this->addChild(instructionLabel, 1);

    // 加载所有可用图片
    loadAllImages();

    // 创建图片选择区域
    createImageSelectionArea();

}

void Store::onEnter()
{
    Layer::onEnter();

    ;

   
}

void Store::onExit()
{
    

    // 清理所有子节点
    this->removeAllChildrenWithCleanup(true);

    Layer::onExit();
}

void Store::displayPlayerResources()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建资源显示背景
    auto resourceBg = LayerColor::create(Color4B(0, 0, 0, 150), visibleSize.width, 60);
    resourceBg->setPosition(Vec2(0, visibleSize.height - 60));
    this->addChild(resourceBg, 5);

    // 金币图标
    auto goldIcon = Sprite::create("others/GoldIcon.png");
    if (goldIcon == nullptr) {
        goldIcon = Sprite::create();
        goldIcon->setTextureRect(Rect(0, 0, 50, 50));
        goldIcon->setColor(Color3B::YELLOW);
    }
    goldIcon->setPosition(Vec2(100, visibleSize.height - 30));
    goldIcon->setScale(1.0f);
    this->addChild(goldIcon, 6);

    // 金币数量
    auto goldLabel = Label::createWithTTF(StringUtils::format("%d", g_playerGold),
        "fonts/arial.ttf", 28);
    goldLabel->setPosition(Vec2(160, visibleSize.height - 30));
    goldLabel->setColor(Color3B::YELLOW);
    goldLabel->setTag(GOLD_LABEL_TAG);
    this->addChild(goldLabel, 6);

    // 水晶图标
    auto ElixirIcon = Sprite::create("others/ElixirIcon.png");
    if (ElixirIcon == nullptr) {
        ElixirIcon = Sprite::create();
        ElixirIcon->setTextureRect(Rect(0, 0, 50, 50));
        ElixirIcon->setColor(Color3B::BLUE);
    }
    ElixirIcon->setPosition(Vec2(280, visibleSize.height - 30));
    ElixirIcon->setScale(1.0f);
    this->addChild(ElixirIcon, 6);

    // 水晶数量
    auto ElixirLabel = Label::createWithTTF(StringUtils::format("%d", g_playerCrystal),
        "fonts/arial.ttf", 28);
    ElixirLabel->setPosition(Vec2(340, visibleSize.height - 30));
    ElixirLabel->setColor(Color3B::BLUE);
    ElixirLabel->setTag(CRYSTAL_LABEL_TAG);
    this->addChild(ElixirLabel, 6);
}

void Store::loadAllImages()
{
    // 加载1(1)到1(5)的图片
    for (int i = 1; i <= 8; i++) {
        std::string filename = StringUtils::format("others/1 (%d).png", i);
        auto sprite = Sprite::create(filename);
        if (sprite != nullptr) {
            allImages.pushBack(sprite);
        }
        else {
            // 如果图片加载失败，创建一个占位符
            auto placeholder = Sprite::create();
            placeholder->setTextureRect(Rect(0, 0, 100, 100));
            placeholder->setColor(Color3B(100 + i * 30, 100, 100 + i * 20));

            allImages.pushBack(placeholder);
        }
    }

    // 记录每个精灵对应的文件名
    for (int i = 0; i < allImages.size(); i++) {
        allImages.at(i)->setName(StringUtils::format("image_%d", i + 1));
    }
}

void Store::createImageSelectionArea()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建图片展示区域
    createImageGallery();

    // 添加触摸事件监听器
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(Store::onTouchBegan, this);
    listener->onTouchEnded = CC_CALLBACK_2(Store::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void Store::createImageGallery()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 添加当前页码显示
    currentPageLabel = Label::createWithTTF("1/3", "fonts/arial.ttf", 20);
    currentPageLabel->setPosition(Vec2(visibleSize.width / 2, 380));
    currentPageLabel->setColor(Color3B::WHITE);
    this->addChild(currentPageLabel, 6);

    // 创建向左翻页箭头
    auto leftArrow = Sprite::create("arrow_left.png");
    if (leftArrow == nullptr) {
        leftArrow = Sprite::create();
        leftArrow->setTextureRect(Rect(0, 0, 40, 40));
        leftArrow->setColor(Color3B::GRAY);

        // 绘制三角形箭头
        auto triangle = DrawNode::create();
        Vec2 vertices[] = { Vec2(30, 10), Vec2(10, 20), Vec2(30, 30) };
        triangle->drawPolygon(vertices, 3, Color4F(1, 1, 1, 1), 0, Color4F(1, 1, 1, 1));
        leftArrow->addChild(triangle);
    }
    leftArrow->setPosition(Vec2(30, 250));
    leftArrow->setTag(LEFT_ARROW_TAG);
    this->addChild(leftArrow, 6);

    // 创建向右翻页箭头
    auto rightArrow = Sprite::create("arrow_right.png");
    if (rightArrow == nullptr) {
        rightArrow = Sprite::create();
        rightArrow->setTextureRect(Rect(0, 0, 40, 40));
        rightArrow->setColor(Color3B::GRAY);

        // 绘制三角形箭头
        auto triangle = DrawNode::create();
        Vec2 vertices[] = { Vec2(10, 10), Vec2(30, 20), Vec2(10, 30) };
        triangle->drawPolygon(vertices, 3, Color4F(1, 1, 1, 1), 0, Color4F(1, 1, 1, 1));
        rightArrow->addChild(triangle);
    }
    rightArrow->setPosition(Vec2(visibleSize.width - 30, 250));
    rightArrow->setTag(RIGHT_ARROW_TAG);
    this->addChild(rightArrow, 6);

    // 初始显示第一页
    currentPage = 0;
    updateGalleryDisplay();
}

bool Store::onTouchBegan(Touch* touch, Event* event)
{
    auto touchPos = touch->getLocation();

    // 检查是否点击了翻页箭头
    auto leftArrow = this->getChildByTag(LEFT_ARROW_TAG);
    auto rightArrow = this->getChildByTag(RIGHT_ARROW_TAG);

    if (leftArrow && leftArrow->getBoundingBox().containsPoint(touchPos)) {
        leftArrow->setColor(Color3B(200, 200, 200));
        return true;
    }

    if (rightArrow && rightArrow->getBoundingBox().containsPoint(touchPos)) {
        rightArrow->setColor(Color3B(200, 200, 200));
        return true;
    }

    // 检查是否点击了图片
    for (int i = 0; i < visibleImages.size(); i++) {
        auto sprite = visibleImages.at(i);
        if (sprite && sprite->getBoundingBox().containsPoint(touchPos)) {
            touchedImageIndex = i;
            sprite->runAction(ScaleTo::create(0.1, sprite->getScale() * 1.1));
            return true;
        }
    }

    return false;
}

void Store::updateResourceDisplay()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 更新金币显示
    auto goldLabel = (Label*)this->getChildByTag(GOLD_LABEL_TAG);
    if (goldLabel) {
        goldLabel->setString(StringUtils::format("%d", g_playerGold));
    }

    // 更新水晶显示
    auto crystalLabel = (Label*)this->getChildByTag(CRYSTAL_LABEL_TAG);
    if (crystalLabel) {
        crystalLabel->setString(StringUtils::format("%d", g_playerCrystal));
    }
}

void Store::updateGalleryDisplay()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 移除当前显示的所有图片
    for (auto& sprite : visibleImages) {
        if (sprite) {
            sprite->removeFromParent();
        }
    }
    visibleImages.clear();

    // 计算总页数
    int totalPages = (int)ceil(allImages.size() / 3.0f);
    currentPageLabel->setString(StringUtils::format("%d/%d", currentPage + 1, totalPages));

    // 计算当前页起始索引
    int startIndex = currentPage * 3;

    // 显示当前页的图片（最多3张）
    // 计算中间60%的宽度
    float middleWidth = visibleSize.width * 0.6f;
    // 计算起始X坐标（从左边20%开始）
    float startX = visibleSize.width * 0.2f;
    // 计算每个图片的间距
    float itemSpacing = middleWidth / 3;

    for (int i = 0; i < 3; i++) {
        int imageIndex = startIndex + i;
        if (imageIndex < allImages.size()) {
            // 获取原始精灵
            auto originalSprite = allImages.at(imageIndex);

            // 创建副本
            auto spriteCopy = Sprite::create();
            spriteCopy->setTexture(originalSprite->getTexture());
            spriteCopy->setTextureRect(originalSprite->getTextureRect());
            spriteCopy->setColor(originalSprite->getColor());
            spriteCopy->setContentSize(originalSprite->getContentSize());
            spriteCopy->setName(originalSprite->getName());

            // 设置位置
            float posX = startX + itemSpacing * (i + 0.5f);
            float posY = visibleSize.height * 0.6f;

            // 设置合适的大小
            float maxSize = 200.0f;
            float scale = 1.5f;
            auto contentSize = spriteCopy->getContentSize();
            if (contentSize.width > maxSize || contentSize.height > maxSize) {
                scale = maxSize / MAX(contentSize.width, contentSize.height) * 1.2f;
            }
            spriteCopy->setScale(scale);

            spriteCopy->setPosition(Vec2(posX, posY));
            this->addChild(spriteCopy, 6);
            visibleImages.pushBack(spriteCopy);

            // 添加图片价格显示
            int goldCost = g_imageGoldPrices[imageIndex];
            int crystalCost = g_imageCrystalPrices[imageIndex];

            // 金币价格
            auto goldCostIcon = Sprite::create("Gold.png");
            if (goldCostIcon == nullptr) {
                goldCostIcon = Sprite::create();
                goldCostIcon->setTextureRect(Rect(0, 0, 25, 25));
                goldCostIcon->setColor(Color3B::YELLOW);
            }
            goldCostIcon->setScale(0.8f);
            goldCostIcon->setPosition(Vec2(-10, -contentSize.height * scale / 2));
            spriteCopy->addChild(goldCostIcon);

            auto goldCostLabel = Label::createWithTTF(StringUtils::format("%d", goldCost),
                "fonts/arial.ttf", 25);
            goldCostLabel->setPosition(Vec2(25, -contentSize.height * scale / 2));
            goldCostLabel->setColor(Color3B::YELLOW);
            spriteCopy->addChild(goldCostLabel);

            // 水晶价格
            auto crystalCostIcon = Sprite::create("others/Crystal.png");
            if (crystalCostIcon == nullptr) {
                crystalCostIcon = Sprite::create();
                crystalCostIcon->setTextureRect(Rect(0, 0, 25, 25));
                crystalCostIcon->setColor(Color3B::BLUE);
            }
            crystalCostIcon->setScale(0.8f);
            crystalCostIcon->setPosition(Vec2(75, -contentSize.height * scale / 2));
            spriteCopy->addChild(crystalCostIcon);

            auto crystalCostLabel = Label::createWithTTF(StringUtils::format("%d", crystalCost),
                "fonts/arial.ttf", 18);
            crystalCostLabel->setPosition(Vec2(105, -contentSize.height * scale / 2));
            crystalCostLabel->setColor(Color3B::BLUE);
            spriteCopy->addChild(crystalCostLabel);

        }
    }
}

void Store::onTouchEnded(Touch* touch, Event* event)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto touchPos = touch->getLocation();

    auto leftArrow = this->getChildByTag(LEFT_ARROW_TAG);
    auto rightArrow = this->getChildByTag(RIGHT_ARROW_TAG);

    if (leftArrow) leftArrow->setColor(Color3B::GRAY);
    if (rightArrow) rightArrow->setColor(Color3B::GRAY);

    for (auto& sprite : visibleImages) {
        sprite->stopAllActions();
        sprite->runAction(ScaleTo::create(0.1, sprite->getScale()));
    }

    // 检查是否点击了左箭头
    if (leftArrow && leftArrow->getBoundingBox().containsPoint(touchPos)) {
        if (currentPage > 0) {
            currentPage--;
            updateGalleryDisplay();
            // 播放点击音效
            AudioEngine::play2d("click.mp3", false, 0.6f);
        }
        return;
    }

    // 检查是否点击了右箭头
    if (rightArrow && rightArrow->getBoundingBox().containsPoint(touchPos)) {
        int totalPages = (int)ceil(allImages.size() / 3.0f);
        if (currentPage < totalPages - 1) {
            currentPage++;
            updateGalleryDisplay();
            // 播放点击音效
            AudioEngine::play2d("click.mp3", false, 0.6f);
        }
        return;
    }

    // 检查是否点击了图片
    for (int i = 0; i < visibleImages.size(); i++) {
        auto sprite = visibleImages.at(i);
        if (sprite && sprite->getBoundingBox().containsPoint(touchPos)) {
            int actualIndex = currentPage * 3 + i;
            if (actualIndex < allImages.size()) {
                int goldCost = g_imageGoldPrices[actualIndex];
                int crystalCost = g_imageCrystalPrices[actualIndex];

                if (g_playerGold >= goldCost && g_playerCrystal >= crystalCost) {
                    g_playerGold -= goldCost;
                    g_playerCrystal -= crystalCost;

                    auto originalSprite = allImages.at(actualIndex);

                    g_selectedImageIndex = actualIndex;
                    g_selectedImageRect = originalSprite->getTextureRect();
                    g_selectedImageColor = originalSprite->getColor();
                    g_selectedTexture = originalSprite->getTexture();
                    g_selectedTextureRect = originalSprite->getTextureRect();
                    g_selectedImageName = originalSprite->getName();

                    // 播放购买音效
                    AudioEngine::play2d("gold.mp3", false, 0.7f);

                    this->setVisible(false);
                }
                else {
                    // 播放点击音效（资源不足时）
                    AudioEngine::play2d("click.mp3", false, 0.8f);

                    auto notEnoughLabel = Label::createWithTTF("资源不足！", "fonts/arial.ttf", 20);
                    notEnoughLabel->setPosition(Vec2(visibleSize.width / 2.0f, 400.0f));
                    notEnoughLabel->setColor(Color3B::RED);
                    this->addChild(notEnoughLabel, 6);

                    notEnoughLabel->runAction(Sequence::create(
                        DelayTime::create(1.5f),
                        FadeOut::create(0.5f),
                        RemoveSelf::create(),
                        nullptr
                    ));
                }
                return;
            }
        }
    }
}

