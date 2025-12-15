#include "HelloWorldScene.h"

USING_NS_CC;

// 全局变量，用于存储从BeachScene选择的图片信息
static int g_selectedImageIndex = -1;
static cocos2d::Rect g_selectedImageRect = cocos2d::Rect::ZERO;
static cocos2d::Color3B g_selectedImageColor = cocos2d::Color3B::WHITE;
static cocos2d::Texture2D* g_selectedTexture = nullptr;
static cocos2d::Rect g_selectedTextureRect = cocos2d::Rect::ZERO;
static std::string g_selectedImageName = "";

// 玩家资源
static int g_playerGold = 100;    // 初始金币
static int g_playerCrystal = 50;  // 初始宝石

// 图片价格定义
static const std::vector<int> g_imageGoldPrices = { 20, 30, 25, 40, 35 };   // 图片的金币价格
static const std::vector<int> g_imageCrystalPrices = { 5, 8, 6, 10, 7 };   // 图片的宝石价格

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

bool HelloWorld::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 设置夏日海滨背景
    auto background = Sprite::create("back.png");
    if (background != nullptr)
    {
        // 设置背景图片适应屏幕大小
        background->setScaleX(visibleSize.width / background->getContentSize().width);
        background->setScaleY(visibleSize.height / background->getContentSize().height);
        background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        this->addChild(background, -1);
    }
    else
    {
        // 如果背景图片加载失败，使用蓝色背景作为备用
        auto fallbackBackground = LayerColor::create(Color4B(0, 150, 255, 255),
            visibleSize.width, visibleSize.height);
        this->addChild(fallbackBackground, -1);
    }

    // 在顶部显示玩家资源
    displayPlayerResources();

    // 检查是否有从BeachScene选择的图片
    if (g_selectedImageIndex != -1) {
        // 创建并显示选择的图片
        auto selectedSprite = createSelectedImageSprite();
        if (selectedSprite) {
            // 在屏幕中间放置
            selectedSprite->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
            this->addChild(selectedSprite, 1);

            // 添加动画效果
            selectedSprite->setScale(0);
            selectedSprite->runAction(Sequence::create(
                ScaleTo::create(0.3, 1.2),
                ScaleTo::create(0.2, 1.0),
                nullptr
            ));

            // 添加提示文字
            auto successLabel = Label::createWithTTF("成功添加图片到主场景！", "fonts/arial.ttf", 24);
            successLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
            successLabel->setColor(Color3B::GREEN);
            this->addChild(successLabel, 1);

            // 3秒后淡出提示
            successLabel->runAction(Sequence::create(
                DelayTime::create(3.0f),
                FadeOut::create(0.5f),
                RemoveSelf::create(),
                nullptr
            ));
        }

        // 重置选择状态
        g_selectedImageIndex = -1;
    }

    // 在右下角添加猫图标
    auto catIcon = Sprite::create("house.png");
    if (catIcon == nullptr) {
        // 备用方案：创建简单图标
        catIcon = Sprite::create();
        catIcon->setTextureRect(Rect(0, 0, 100, 100));
        catIcon->setColor(Color3B::WHITE);
    }

    // 设置猫图标在右下角
    float iconSize = 100.0f;
    catIcon->setScale(iconSize / catIcon->getContentSize().width);
    catIcon->setPosition(Vec2(visibleSize.width - iconSize / 2 - 20,
        iconSize / 2 + 20));

    // 添加触摸事件
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [catIcon](Touch* touch, Event* event) {
        if (catIcon->getBoundingBox().containsPoint(touch->getLocation())) {
            catIcon->runAction(ScaleTo::create(0.1, 1.1)); // 点击时放大效果
            return true;
        }
        return false;
        };

    listener->onTouchEnded = [this, catIcon](Touch* touch, Event* event) {
        catIcon->runAction(ScaleTo::create(0.1, 1.0)); // 恢复大小
        if (catIcon->getBoundingBox().containsPoint(touch->getLocation())) {
            // 点击后进入海滩场景
            this->openBeachScene();
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, catIcon);

    // 添加提示文字
    auto tipLabel = Label::createWithTTF("点击右下角的猫进入商店", "fonts/arial.ttf", 24);
    if (tipLabel) {
        tipLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        tipLabel->setColor(Color3B::WHITE);
        this->addChild(tipLabel, 1);
    }

    this->addChild(catIcon, 1);

    return true;
}

void HelloWorld::displayPlayerResources()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();


    // 金币图标
    auto goldIcon = Sprite::create("Gold.png");
    if (goldIcon == nullptr) {
        goldIcon = Sprite::create();
        goldIcon->setTextureRect(Rect(0, 0, 50, 50));  // 从40放大到50
        goldIcon->setColor(Color3B::YELLOW);
    }
    goldIcon->setPosition(Vec2(100, visibleSize.height - 30));
    goldIcon->setScale(1.0f);  // 从0.8放大到1.0
    this->addChild(goldIcon, 6);

    // 金币数量
    auto goldLabel = Label::createWithTTF(StringUtils::format("%d", g_playerGold),
        "fonts/arial.ttf", 28);  // 从24放大到28
    goldLabel->setPosition(Vec2(160, visibleSize.height - 30));  // 从150调整到160
    goldLabel->setColor(Color3B::YELLOW);
    goldLabel->setTag(GOLD_LABEL_TAG);
    this->addChild(goldLabel, 6);

    // 宝石图标
    auto crystalIcon = Sprite::create("Crystal.png");
    if (crystalIcon == nullptr) {
        crystalIcon = Sprite::create();
        crystalIcon->setTextureRect(Rect(0, 0, 50, 50));  // 从40放大到50
        crystalIcon->setColor(Color3B::BLUE);
    }
    crystalIcon->setPosition(Vec2(280, visibleSize.height - 30));  // 从250调整到280
    crystalIcon->setScale(1.0f);  // 从0.8放大到1.0
    this->addChild(crystalIcon, 6);

    // 宝石数量
    auto crystalLabel = Label::createWithTTF(StringUtils::format("%d", g_playerCrystal),
        "fonts/arial.ttf", 28);  // 从24放大到28
    crystalLabel->setPosition(Vec2(340, visibleSize.height - 30));  // 从300调整到340
    crystalLabel->setColor(Color3B::BLUE);
    crystalLabel->setTag(CRYSTAL_LABEL_TAG);
    this->addChild(crystalLabel, 6);
}

cocos2d::Sprite* HelloWorld::createSelectedImageSprite()
{
    if (g_selectedImageIndex == -1) {
        return nullptr;
    }

    Sprite* sprite = nullptr;

    if (g_selectedTexture) {
        // 如果有纹理，使用纹理创建精灵
        sprite = Sprite::create();
        sprite->setTexture(g_selectedTexture);
        sprite->setTextureRect(g_selectedTextureRect);
    }
    else if (!g_selectedImageRect.equals(Rect::ZERO)) {
        // 如果有纹理矩形，使用矩形创建精灵
        sprite = Sprite::create();
        sprite->setTextureRect(g_selectedImageRect);
    }

    if (sprite) {
        // 设置颜色
        sprite->setColor(g_selectedImageColor);

        // 添加标签显示图片编号
        auto label = Label::createWithTTF(StringUtils::format("图片%d", g_selectedImageIndex + 1),
            "fonts/arial.ttf", 20);
        label->setPosition(Vec2(0, -sprite->getContentSize().height / 2 - 20));
        label->setColor(Color3B::YELLOW);
        sprite->addChild(label);

        // 添加点击效果
        auto touchListener = EventListenerTouchOneByOne::create();
        touchListener->onTouchBegan = [sprite](Touch* touch, Event* event) {
            if (sprite->getBoundingBox().containsPoint(touch->getLocation())) {
                sprite->runAction(ScaleTo::create(0.1, 1.1));
                return true;
            }
            return false;
            };

        touchListener->onTouchEnded = [sprite](Touch* touch, Event* event) {
            sprite->runAction(ScaleTo::create(0.1, 1.0));
            };

        touchListener->onTouchMoved = [sprite](Touch* touch, Event* event) {
            auto delta = touch->getDelta();
            sprite->setPosition(sprite->getPosition() + delta);
            };

        Director::getInstance()->getEventDispatcher()
            ->addEventListenerWithSceneGraphPriority(touchListener, sprite);
    }

    return sprite;
}

void HelloWorld::openBeachScene()
{
    // 创建新场景
    auto beachScene = Scene::create();
    auto beachLayer = BeachScene::create();
    beachScene->addChild(beachLayer);

    // 添加过渡效果
    auto transition = TransitionFade::create(0.5, beachScene, Color3B::BLACK);
    Director::getInstance()->replaceScene(transition);
}

// BeachScene 实现
bool BeachScene::init()
{
    if (!Layer::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 初始为纯黑背景
    auto blackBackground = LayerColor::create(Color4B::BLACK, visibleSize.width, visibleSize.height);
    this->addChild(blackBackground, -1);
    
    // 在顶部显示玩家资源
    displayPlayerResources();

    // 添加返回按钮
    auto backButton = Sprite::create("back_button.png");
    if (backButton == nullptr) {
        backButton = Sprite::create();
        backButton->setTextureRect(Rect(0, 0, 60, 60));
        backButton->setColor(Color3B::RED);

        // 添加"X"标记
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
            backButton->runAction(ScaleTo::create(0.1, 0.9)); // 点击时缩小效果
            return true;
        }
        return false;
        };

    backListener->onTouchEnded = [this, backButton](Touch* touch, Event* event) {
        backButton->runAction(ScaleTo::create(0.1, 0.8)); // 恢复大小

        if (backButton->getBoundingBox().containsPoint(touch->getLocation())) {
            // 返回主场景
            auto mainScene = HelloWorld::createScene();
            auto transition = TransitionFade::create(0.5, mainScene, Color3B::BLACK);
            Director::getInstance()->replaceScene(transition);
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(backListener, backButton);
    this->addChild(backButton, 10);

    // 添加提示文字
    auto instructionLabel = Label::createWithTTF("点击图片购买并返回主场景", "fonts/arial.ttf", 24);
    instructionLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 150));
    instructionLabel->setColor(Color3B::YELLOW);
    this->addChild(instructionLabel, 1);

    // 加载所有可用的图片
    loadAllImages();

    // 创建图片选择区域
    createImageSelectionArea();

    return true;
}

void BeachScene::displayPlayerResources()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建资源显示背景
    auto resourceBg = LayerColor::create(Color4B(0, 0, 0, 150), visibleSize.width, 60);
    resourceBg->setPosition(Vec2(0, visibleSize.height - 60));
    this->addChild(resourceBg, 5);

    // 金币图标
    auto goldIcon = Sprite::create("Gold.png");
    if (goldIcon == nullptr) {
        goldIcon = Sprite::create();
        goldIcon->setTextureRect(Rect(0, 0, 50, 50));  // 从40放大到50
        goldIcon->setColor(Color3B::YELLOW);
    }
    goldIcon->setPosition(Vec2(100, visibleSize.height - 30));
    goldIcon->setScale(1.0f);  // 从0.8放大到1.0
    this->addChild(goldIcon, 6);

    // 金币数量
    auto goldLabel = Label::createWithTTF(StringUtils::format("%d", g_playerGold),
        "fonts/arial.ttf", 28);  // 从24放大到28
    goldLabel->setPosition(Vec2(160, visibleSize.height - 30));  // 从150调整到160
    goldLabel->setColor(Color3B::YELLOW);
    goldLabel->setTag(GOLD_LABEL_TAG);
    this->addChild(goldLabel, 6);

    // 宝石图标
    auto crystalIcon = Sprite::create("Crystal.png");
    if (crystalIcon == nullptr) {
        crystalIcon = Sprite::create();
        crystalIcon->setTextureRect(Rect(0, 0, 50, 50));  // 从40放大到50
        crystalIcon->setColor(Color3B::BLUE);
    }
    crystalIcon->setPosition(Vec2(280, visibleSize.height - 30));  // 从250调整到280
    crystalIcon->setScale(1.0f);  // 从0.8放大到1.0
    this->addChild(crystalIcon, 6);

    // 宝石数量
    auto crystalLabel = Label::createWithTTF(StringUtils::format("%d", g_playerCrystal),
        "fonts/arial.ttf", 28);  // 从24放大到28
    crystalLabel->setPosition(Vec2(340, visibleSize.height - 30));  // 从300调整到340
    crystalLabel->setColor(Color3B::BLUE);
    crystalLabel->setTag(CRYSTAL_LABEL_TAG);
    this->addChild(crystalLabel, 6);
}

void BeachScene::loadAllImages()
{
    // 加载1(1)到1(5)五张图片
    for (int i = 1; i <= 5; i++) {
        std::string filename = StringUtils::format("1 (%d).png", i);
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

void BeachScene::createImageSelectionArea()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建图片展示区域
    createImageGallery();

    // 创建触摸事件监听器
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(BeachScene::onTouchBegan, this);
    listener->onTouchEnded = CC_CALLBACK_2(BeachScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void BeachScene::createImageGallery()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 添加说明文字
    auto instructionLabel = Label::createWithTTF("点击图片购买，将自动返回主场景", "fonts/arial.ttf", 20);
    instructionLabel->setPosition(Vec2(visibleSize.width / 2, 30));
    instructionLabel->setColor(Color3B::WHITE);
    this->addChild(instructionLabel, 6);

    // 添加当前页码显示
    currentPageLabel = Label::createWithTTF("1/2", "fonts/arial.ttf", 20);
    currentPageLabel->setPosition(Vec2(visibleSize.width / 2, 380));  // 从330调整到380
    currentPageLabel->setColor(Color3B::WHITE);
    this->addChild(currentPageLabel, 6);

    // 添加向左翻页箭头
    auto leftArrow = Sprite::create("arrow_left.png");
    if (leftArrow == nullptr) {
        leftArrow = Sprite::create();
        leftArrow->setTextureRect(Rect(0, 0, 40, 40));
        leftArrow->setColor(Color3B::GRAY);

        // 添加三角形箭头
        auto triangle = DrawNode::create();
        Vec2 vertices[] = { Vec2(30, 10), Vec2(10, 20), Vec2(30, 30) };
        triangle->drawPolygon(vertices, 3, Color4F(1, 1, 1, 1), 0, Color4F(1, 1, 1, 1));
        leftArrow->addChild(triangle);
    }
    leftArrow->setPosition(Vec2(30, 250));  // 从200调整到250
    leftArrow->setTag(LEFT_ARROW_TAG);
    this->addChild(leftArrow, 6);

    // 添加向右翻页箭头
    auto rightArrow = Sprite::create("arrow_right.png");
    if (rightArrow == nullptr) {
        rightArrow = Sprite::create();
        rightArrow->setTextureRect(Rect(0, 0, 40, 40));
        rightArrow->setColor(Color3B::GRAY);

        // 添加三角形箭头
        auto triangle = DrawNode::create();
        Vec2 vertices[] = { Vec2(10, 10), Vec2(30, 20), Vec2(10, 30) };
        triangle->drawPolygon(vertices, 3, Color4F(1, 1, 1, 1), 0, Color4F(1, 1, 1, 1));
        rightArrow->addChild(triangle);
    }
    rightArrow->setPosition(Vec2(visibleSize.width - 30, 250));  // 从200调整到250
    rightArrow->setTag(RIGHT_ARROW_TAG);
    this->addChild(rightArrow, 6);

    // 初始显示第一页
    currentPage = 0;
    updateGalleryDisplay();
}

bool BeachScene::onTouchBegan(Touch* touch, Event* event)
{
    auto touchPos = touch->getLocation();

    // 检查是否点击了翻页箭头
    auto leftArrow = this->getChildByTag(LEFT_ARROW_TAG);
    auto rightArrow = this->getChildByTag(RIGHT_ARROW_TAG);

    if (leftArrow && leftArrow->getBoundingBox().containsPoint(touchPos)) {
        leftArrow->setColor(Color3B(200, 200, 200)); // 点击效果
        return true;
    }

    if (rightArrow && rightArrow->getBoundingBox().containsPoint(touchPos)) {
        rightArrow->setColor(Color3B(200, 200, 200)); // 点击效果
        return true;
    }

    // 检查是否点击了图片
    for (int i = 0; i < visibleImages.size(); i++) {
        auto sprite = visibleImages.at(i);
        if (sprite && sprite->getBoundingBox().containsPoint(touchPos)) {
            touchedImageIndex = i;
            sprite->runAction(ScaleTo::create(0.1, sprite->getScale() * 1.1)); // 点击效果
            return true;
        }
    }

    return false;
}

void BeachScene::onTouchEnded(Touch* touch, Event* event)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();  // 修复：添加visibleSize定义
    auto touchPos = touch->getLocation();

    // 恢复箭头颜色
    auto leftArrow = this->getChildByTag(LEFT_ARROW_TAG);
    auto rightArrow = this->getChildByTag(RIGHT_ARROW_TAG);

    if (leftArrow) leftArrow->setColor(Color3B::GRAY);
    if (rightArrow) rightArrow->setColor(Color3B::GRAY);

    // 恢复被点击图片的大小
    for (auto& sprite : visibleImages) {
        sprite->stopAllActions();
        sprite->runAction(ScaleTo::create(0.1, sprite->getScale())); // 恢复原始大小
    }

    // 检查是否点击了左箭头
    if (leftArrow && leftArrow->getBoundingBox().containsPoint(touchPos)) {
        if (currentPage > 0) {
            currentPage--;
            updateGalleryDisplay();
        }
        return;
    }

    // 检查是否点击了右箭头
    if (rightArrow && rightArrow->getBoundingBox().containsPoint(touchPos)) {
        int totalPages = (int)ceil(allImages.size() / 3.0f);
        if (currentPage < totalPages - 1) {
            currentPage++;
            updateGalleryDisplay();
        }
        return;
    }

    // 检查是否点击了图片
    for (int i = 0; i < visibleImages.size(); i++) {
        auto sprite = visibleImages.at(i);
        if (sprite && sprite->getBoundingBox().containsPoint(touchPos)) {
            // 保存选择的图片信息
            int actualIndex = currentPage * 3 + i;
            if (actualIndex < allImages.size()) {
                // 检查玩家是否有足够的资源购买
                int goldCost = g_imageGoldPrices[actualIndex];
                int crystalCost = g_imageCrystalPrices[actualIndex];

                if (g_playerGold >= goldCost && g_playerCrystal >= crystalCost) {
                    // 扣除资源
                    g_playerGold -= goldCost;
                    g_playerCrystal -= crystalCost;

                    auto originalSprite = allImages.at(actualIndex);

                    // 保存图片信息到全局变量
                    g_selectedImageIndex = actualIndex;
                    g_selectedImageRect = originalSprite->getTextureRect();
                    g_selectedImageColor = originalSprite->getColor();
                    g_selectedTexture = originalSprite->getTexture();
                    g_selectedTextureRect = originalSprite->getTextureRect();
                    g_selectedImageName = originalSprite->getName();

                    // 添加购买成功动画
                    sprite->runAction(Sequence::create(
                        ScaleTo::create(0.1, sprite->getScale() * 1.2),
                        ScaleTo::create(0.1, sprite->getScale() * 0.8),
                        ScaleTo::create(0.1, sprite->getScale()),
                        CallFunc::create([this]() {
                            // 更新资源显示
                            updateResourceDisplay();

                            // 返回主场景
                            auto mainScene = HelloWorld::createScene();
                            auto transition = TransitionFade::create(0.5, mainScene, Color3B::BLACK);
                            Director::getInstance()->replaceScene(transition);
                            }),
                        nullptr
                    ));
                }
                else {
                    // 资源不足，显示提示
                    auto notEnoughLabel = Label::createWithTTF("资源不足！", "fonts/arial.ttf", 20);
                    notEnoughLabel->setPosition(Vec2(visibleSize.width / 2.0f, 400.0f));  // 从350调整到400，并修复类型转换
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

void BeachScene::updateResourceDisplay()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 更新金币显示
    auto goldLabel = (Label*)this->getChildByTag(GOLD_LABEL_TAG);
    if (goldLabel) {
        goldLabel->setString(StringUtils::format("%d", g_playerGold));
    }

    // 更新宝石显示
    auto crystalLabel = (Label*)this->getChildByTag(CRYSTAL_LABEL_TAG);
    if (crystalLabel) {
        crystalLabel->setString(StringUtils::format("%d", g_playerCrystal));
    }
}

void BeachScene::updateGalleryDisplay()
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

    // 计算当前页的起始索引
    int startIndex = currentPage * 3;

    // 显示当前页的图片（最多3张）
    float itemWidth = visibleSize.width / 3;

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
            float posX = itemWidth * (i + 0.5f);
            float posY = visibleSize.height * 0.6f;

            // 设置精灵大小
            float maxSize = 150.0f;  // 从120增加到150，放大图片
            float scale = 1.2f;  // 从1.0增加到1.2，整体放大
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
                goldCostIcon->setTextureRect(Rect(0, 0, 25, 25));  // 从20增加到25
                goldCostIcon->setColor(Color3B::YELLOW);
            }
            goldCostIcon->setScale(0.6f);  // 从0.5增加到0.6
            goldCostIcon->setPosition(Vec2(-30, -contentSize.height * scale / 2 - 50));  // 从-40调整到-50
            spriteCopy->addChild(goldCostIcon);

            auto goldCostLabel = Label::createWithTTF(StringUtils::format("%d", goldCost),
                "fonts/arial.ttf", 18);  // 从16增加到18
            goldCostLabel->setPosition(Vec2(-5, -contentSize.height * scale / 2 - 50));  // 从-40调整到-50
            goldCostLabel->setColor(Color3B::YELLOW);
            spriteCopy->addChild(goldCostLabel);

            // 宝石价格
            auto crystalCostIcon = Sprite::create("Crystal.png");
            if (crystalCostIcon == nullptr) {
                crystalCostIcon = Sprite::create();
                crystalCostIcon->setTextureRect(Rect(0, 0, 25, 25));  // 从20增加到25
                crystalCostIcon->setColor(Color3B::BLUE);
            }
            crystalCostIcon->setScale(0.6f);  // 从0.5增加到0.6
            crystalCostIcon->setPosition(Vec2(30, -contentSize.height * scale / 2 - 50));  // 从-40调整到-50
            spriteCopy->addChild(crystalCostIcon);

            auto crystalCostLabel = Label::createWithTTF(StringUtils::format("%d", crystalCost),
                "fonts/arial.ttf", 18);  // 从16增加到18
            crystalCostLabel->setPosition(Vec2(55, -contentSize.height * scale / 2 - 50));  // 从-40调整到-50
            crystalCostLabel->setColor(Color3B::BLUE);
            spriteCopy->addChild(crystalCostLabel);

            // 添加图片编号标签
            auto label = Label::createWithTTF(StringUtils::format("图片%d", imageIndex + 1),
                "fonts/arial.ttf", 22);  // 从20增加到22
            label->setPosition(Vec2(0, -contentSize.height * scale / 2 - 30));  // 从-20调整到-30
            label->setColor(Color3B::YELLOW);
            label->setTag(LABEL_TAG);
            spriteCopy->addChild(label);
        }
    }
}