#include "people.h"

// 静态成员初始化
PopulationManager* PopulationManager::s_instance = nullptr;

// ============ Character 类实现 ============
Character::Character(CharacterType type, const std::string& name, const std::string& image)
    : m_type(type)
    , m_name(name)
    , m_image(image)
    , m_count(0)
{
}

// ============ PopulationManager 类实现 ============
PopulationManager::PopulationManager()
    : m_totalPopulation(0)
    , m_warrior(nullptr)
    , m_archer(nullptr)
    , m_mage(nullptr)
    , m_healer(nullptr)
{
    // 创建四种人物
    m_warrior = new Character(CharacterType::WARRIOR, "野蛮人", "1.png");
    m_archer = new Character(CharacterType::ARCHER, "弓箭手", "2.png");
    m_mage = new Character(CharacterType::MAGE, "巨人", "3.png");
    m_healer = new Character(CharacterType::HEALER, "哥布林", "4.png");

    // 初始化每种人物的消耗
    m_characterCosts[CharacterType::WARRIOR] = 1;
    m_characterCosts[CharacterType::ARCHER] = 2;
    m_characterCosts[CharacterType::MAGE] = 5;
    m_characterCosts[CharacterType::HEALER] = 1;

    CCLOG("PopulationManager created");
}

PopulationManager::~PopulationManager()
{
    delete m_warrior;
    delete m_archer;
    delete m_mage;
    delete m_healer;

    CCLOG("PopulationManager destroyed");
}

PopulationManager* PopulationManager::getInstance()
{
    if (!s_instance) {
        s_instance = new PopulationManager();
    }
    return s_instance;
}

void PopulationManager::destroyInstance()
{
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

void PopulationManager::initialize(int totalPopulation)
{
    m_totalPopulation = totalPopulation;
    resetAllCharacters();
    CCLOG("PopulationManager initialized with %d total population", totalPopulation);
}

int PopulationManager::getCharacterCost(CharacterType type) const
{
    auto it = m_characterCosts.find(type);
    if (it != m_characterCosts.end()) {
        return it->second;
    }
    return 1;
}

int PopulationManager::getAssignedPopulation() const
{
    int total = 0;
    total += m_warrior->getCount() * getCharacterCost(CharacterType::WARRIOR);
    total += m_archer->getCount() * getCharacterCost(CharacterType::ARCHER);
    total += m_mage->getCount() * getCharacterCost(CharacterType::MAGE);
    total += m_healer->getCount() * getCharacterCost(CharacterType::HEALER);
    return total;
}

int PopulationManager::getAvailablePopulation() const
{
    return m_totalPopulation - getAssignedPopulation();
}

bool PopulationManager::assignCharacter(CharacterType type, int count)
{
    if (count <= 0) {
        CCLOG("Error: Cannot assign zero or negative count");
        return false;
    }

    if (!isValidAssignment(type, count)) {
        CCLOG("Error: Invalid assignment for character type %d", (int)type);
        return false;
    }

    Character* character = getCharacter(type);
    if (character) {
        int actualCost = getCharacterCost(type) * count;
        character->addCount(count);
        CCLOG("Assigned %d %s (cost: %d population), total assigned: %d",
            count, character->getName().c_str(), actualCost, getAssignedPopulation());
        return true;
    }

    return false;
}

bool PopulationManager::unassignCharacter(CharacterType type, int count)
{
    if (count <= 0) {
        CCLOG("Error: Cannot unassign zero or negative count");
        return false;
    }

    Character* character = getCharacter(type);
    if (character && character->getCount() >= count) {
        int recoveredCost = getCharacterCost(type) * count;
        character->setCount(character->getCount() - count);
        CCLOG("Unassigned %d %s (recovered: %d population), total assigned: %d",
            count, character->getName().c_str(), recoveredCost, getAssignedPopulation());
        return true;
    }

    CCLOG("Error: Cannot unassign %d %s, only have %d",
        count, character ? character->getName().c_str() : "unknown",
        character ? character->getCount() : 0);
    return false;
}

int PopulationManager::getCharacterCount(CharacterType type) const
{
    const Character* character = getCharacter(type);
    return character ? character->getCount() : 0;
}

void PopulationManager::resetAllCharacters()
{
    m_warrior->resetCount();
    m_archer->resetCount();
    m_mage->resetCount();
    m_healer->resetCount();
    CCLOG("All characters reset");
}

bool PopulationManager::isValidAssignment(CharacterType type, int count) const
{
    if (count <= 0) {
        return false;
    }

    Character* character = getCharacter(type);
    if (!character) {
        return false;
    }

    int cost = getCharacterCost(type) * count;
    int available = getAvailablePopulation();

    if (available < cost) {
        CCLOG("Error: Not enough available population. Need: %d, Available: %d",
            cost, available);
        return false;
    }

    return true;
}

bool PopulationManager::isValidUnassignment(CharacterType type, int count) const
{
    if (count <= 0) {
        return false;
    }

    Character* character = getCharacter(type);
    if (!character) {
        return false;
    }

    if (character->getCount() < count) {
        CCLOG("Error: Not enough %s to unassign. Need: %d, Have: %d",
            character->getName().c_str(), count, character->getCount());
        return false;
    }

    return true;
}

std::vector<Character*> PopulationManager::getAllCharacters() const
{
    return { m_warrior, m_archer, m_mage, m_healer };
}

Character* PopulationManager::getCharacter(CharacterType type) const
{
    switch (type) {
    case CharacterType::WARRIOR: return m_warrior;
    case CharacterType::ARCHER:  return m_archer;
    case CharacterType::MAGE:    return m_mage;
    case CharacterType::HEALER:  return m_healer;
    default: return nullptr;
    }
}

void PopulationManager::saveAssignment()
{
    UserDefault* userDefault = UserDefault::getInstance();
    userDefault->setIntegerForKey("population_total", m_totalPopulation);
    userDefault->setIntegerForKey("character_warrior", m_warrior->getCount());
    userDefault->setIntegerForKey("character_archer", m_archer->getCount());
    userDefault->setIntegerForKey("character_mage", m_mage->getCount());
    userDefault->setIntegerForKey("character_healer", m_healer->getCount());
    userDefault->flush();
    CCLOG("Population assignment saved");
}

void PopulationManager::loadAssignment()
{
    UserDefault* userDefault = UserDefault::getInstance();
    m_totalPopulation = userDefault->getIntegerForKey("population_total", 0);
    m_warrior->setCount(userDefault->getIntegerForKey("character_warrior", 0));
    m_archer->setCount(userDefault->getIntegerForKey("character_archer", 0));
    m_mage->setCount(userDefault->getIntegerForKey("character_mage", 0));
    m_healer->setCount(userDefault->getIntegerForKey("character_healer", 0));
    CCLOG("Population assignment loaded: total=%d", m_totalPopulation);
}

// ============ PopulationScene 场景实现 ============
Scene* PopulationScene::createScene(int totalPopulation)
{
    auto scene = Scene::create();
    auto layer = PopulationScene::create();
    if (layer) {
        layer->setTotalPopulation(totalPopulation);
        auto populationMgr = PopulationManager::getInstance();
        populationMgr->initialize(totalPopulation);
        scene->addChild(layer);
    }
    return scene;
}

bool PopulationScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    m_populationMgr = PopulationManager::getInstance();
    if (m_totalPopulation > 0) {
        m_populationMgr->initialize(m_totalPopulation);
    }
    setupUI();

    return true;
}

void PopulationScene::onEnter()
{
    Scene::onEnter();

    if (m_totalPopulation > 0) {
        m_populationMgr->initialize(m_totalPopulation);
        updateUI();
    }
    else {
        m_populationMgr->loadAssignment();
        m_totalPopulation = m_populationMgr->getTotalPopulation();
        updateUI();
    }

    CCLOG("PopulationScene onEnter, total population: %d", m_totalPopulation);
}

void PopulationScene::setupUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 背景
    auto background = LayerColor::create(Color4B(50, 50, 100, 255), visibleSize.width, visibleSize.height);
    this->addChild(background, -1);

    // 标题
    auto titleLabel = Label::createWithTTF("POPULAR SYSTEM", "fonts/arial.ttf", 36);
    titleLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 50));
    titleLabel->setColor(Color3B::YELLOW);
    this->addChild(titleLabel, 1);

    // 总人口显示
    int currentTotal = m_populationMgr->getTotalPopulation();
    m_totalLabel = Label::createWithTTF(StringUtils::format("TOTLE: %d", currentTotal),
        "fonts/arial.ttf", 28);
    m_totalLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 120));
    m_totalLabel->setColor(Color3B::GREEN);
    this->addChild(m_totalLabel, 1);

    // 可用人口显示
    m_availableLabel = Label::createWithTTF("TOTLE: 0", "fonts/arial.ttf", 24);
    m_availableLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 160));
    m_availableLabel->setColor(Color3B::WHITE);
    this->addChild(m_availableLabel, 1);

    // 创建人物显示区域
    float startY = visibleSize.height * 0.6f;
    float spacingX = visibleSize.width / 5;

    // 野蛮人
    m_warriorSprite = Sprite::create("1.png");
    if (!m_warriorSprite) {
        m_warriorSprite = Sprite::create();
        m_warriorSprite->setTextureRect(Rect(0, 0, 100, 100));
        m_warriorSprite->setColor(Color3B::RED);
    }
    m_warriorSprite->setPosition(Vec2(spacingX, startY));
    m_warriorSprite->setScale(1.0f);
    this->addChild(m_warriorSprite, 1);

    // 显示消耗
    auto warriorName = Label::createWithTTF("野蛮人(消耗:1)", "fonts/arial.ttf", 20);
    warriorName->setPosition(Vec2(spacingX, startY - 80));
    warriorName->setColor(Color3B::RED);
    this->addChild(warriorName, 1);

    m_warriorLabel = Label::createWithTTF("0", "fonts/arial.ttf", 24);
    m_warriorLabel->setPosition(Vec2(spacingX, startY - 110));
    m_warriorLabel->setColor(Color3B::WHITE);
    this->addChild(m_warriorLabel, 1);

    // 弓箭手
    m_archerSprite = Sprite::create("2.png");
    if (!m_archerSprite) {
        m_archerSprite = Sprite::create();
        m_archerSprite->setTextureRect(Rect(0, 0, 100, 100));
        m_archerSprite->setColor(Color3B::GREEN);
    }
    m_archerSprite->setPosition(Vec2(spacingX * 2, startY));
    m_archerSprite->setScale(1.0f);
    this->addChild(m_archerSprite, 1);

    auto archerName = Label::createWithTTF("弓箭手(消耗:2)", "fonts/arial.ttf", 20);
    archerName->setPosition(Vec2(spacingX * 2, startY - 80));
    archerName->setColor(Color3B::GREEN);
    this->addChild(archerName, 1);

    m_archerLabel = Label::createWithTTF("0", "fonts/arial.ttf", 24);
    m_archerLabel->setPosition(Vec2(spacingX * 2, startY - 110));
    m_archerLabel->setColor(Color3B::WHITE);
    this->addChild(m_archerLabel, 1);

    // 巨人
    m_mageSprite = Sprite::create("3.png");
    if (!m_mageSprite) {
        m_mageSprite = Sprite::create();
        m_mageSprite->setTextureRect(Rect(0, 0, 100, 100));
        m_mageSprite->setColor(Color3B::BLUE);
    }
    m_mageSprite->setPosition(Vec2(spacingX * 3, startY));
    m_mageSprite->setScale(1.0f);
    this->addChild(m_mageSprite, 1);

    auto mageName = Label::createWithTTF("巨人(消耗:5)", "fonts/arial.ttf", 20);
    mageName->setPosition(Vec2(spacingX * 3, startY - 80));
    mageName->setColor(Color3B::BLUE);
    this->addChild(mageName, 1);

    m_mageLabel = Label::createWithTTF("0", "fonts/arial.ttf", 24);
    m_mageLabel->setPosition(Vec2(spacingX * 3, startY - 110));
    m_mageLabel->setColor(Color3B::WHITE);
    this->addChild(m_mageLabel, 1);

    // 哥布林
    m_healerSprite = Sprite::create("4.png");
    if (!m_healerSprite) {
        m_healerSprite = Sprite::create();
        m_healerSprite->setTextureRect(Rect(0, 0, 100, 100));
        m_healerSprite->setColor(Color3B::MAGENTA);
    }
    m_healerSprite->setPosition(Vec2(spacingX * 4, startY));
    m_healerSprite->setScale(1.0f);
    this->addChild(m_healerSprite, 1);

    auto healerName = Label::createWithTTF("哥布林(消耗:1)", "fonts/arial.ttf", 20);
    healerName->setPosition(Vec2(spacingX * 4, startY - 80));
    healerName->setColor(Color3B::MAGENTA);
    this->addChild(healerName, 1);

    m_healerLabel = Label::createWithTTF("0", "fonts/arial.ttf", 24);
    m_healerLabel->setPosition(Vec2(spacingX * 4, startY - 110));
    m_healerLabel->setColor(Color3B::WHITE);
    this->addChild(m_healerLabel, 1);

    // 添加人物鼠标点击事件（左键增加，右键减少）
    auto warriorListener = EventListenerMouse::create();
    warriorListener->onMouseDown = [this](EventMouse* event) {
        if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            if (m_warriorSprite->getBoundingBox().containsPoint(event->getLocationInView())) {
                onCharacterClicked(CharacterType::WARRIOR, 1);  // 左键增加
                return;
            }
        }
        else if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            if (m_warriorSprite->getBoundingBox().containsPoint(event->getLocationInView())) {
                onCharacterRightClicked(CharacterType::WARRIOR, 1);  // 右键减少
                return;
            }
        }
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(warriorListener, m_warriorSprite);

    auto archerListener = EventListenerMouse::create();
    archerListener->onMouseDown = [this](EventMouse* event) {
        if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            if (m_archerSprite->getBoundingBox().containsPoint(event->getLocationInView())) {
                onCharacterClicked(CharacterType::ARCHER, 1);
                return;
            }
        }
        else if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            if (m_archerSprite->getBoundingBox().containsPoint(event->getLocationInView())) {
                onCharacterRightClicked(CharacterType::ARCHER, 1);
                return;
            }
        }
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(archerListener, m_archerSprite);

    auto mageListener = EventListenerMouse::create();
    mageListener->onMouseDown = [this](EventMouse* event) {
        if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            if (m_mageSprite->getBoundingBox().containsPoint(event->getLocationInView())) {
                onCharacterClicked(CharacterType::MAGE, 1);
                return;
            }
        }
        else if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            if (m_mageSprite->getBoundingBox().containsPoint(event->getLocationInView())) {
                onCharacterRightClicked(CharacterType::MAGE, 1);
                return;
            }
        }
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mageListener, m_mageSprite);

    auto healerListener = EventListenerMouse::create();
    healerListener->onMouseDown = [this](EventMouse* event) {
        if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            if (m_healerSprite->getBoundingBox().containsPoint(event->getLocationInView())) {
                onCharacterClicked(CharacterType::HEALER, 1);
                return;
            }
        }
        else if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            if (m_healerSprite->getBoundingBox().containsPoint(event->getLocationInView())) {
                onCharacterRightClicked(CharacterType::HEALER, 1);
                return;
            }
        }
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(healerListener, m_healerSprite);

    // 添加动画效果
    auto rotateAction = RotateBy::create(0.5f, 10);
    auto rotateBack = RotateBy::create(0.5f, -10);
    auto sequence = Sequence::create(rotateAction, rotateBack, nullptr);
    auto repeat = RepeatForever::create(sequence);

    m_warriorSprite->runAction(repeat->clone());
    m_archerSprite->runAction(repeat->clone());
    m_mageSprite->runAction(repeat->clone());
    m_healerSprite->runAction(repeat->clone());

    // 战斗按钮
    MenuItem* fightButton = nullptr;
    if (FileUtils::getInstance()->isFileExist("fighting.png")) {
        fightButton = MenuItemImage::create("fighting.png", "fighting.png",
            CC_CALLBACK_1(PopulationScene::onFightClicked, this));
    }
    else {
        fightButton = MenuItemLabel::create(
            Label::createWithTTF("开始战斗", "fonts/arial.ttf", 30),
            CC_CALLBACK_1(PopulationScene::onFightClicked, this));
    }
    fightButton->setPosition(Vec2(visibleSize.width / 2, 120));
    fightButton->setScale(1.5f);

    // 返回按钮
    MenuItem* backButton = nullptr;
    if (FileUtils::getInstance()->isFileExist("button.png")) {
        backButton = MenuItemImage::create("button.png", "button.png",
            CC_CALLBACK_1(PopulationScene::onBackClicked, this));
    }
    else {
        backButton = MenuItemLabel::create(
            Label::createWithTTF("返回", "fonts/arial.ttf", 30),
            CC_CALLBACK_1(PopulationScene::onBackClicked, this));
    }
    backButton->setPosition(Vec2(visibleSize.width / 2, 60));

    auto menu = Menu::create(fightButton, backButton, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    // 提示文字
    auto hintLabel = Label::createWithTTF("左键点击增加人口，右键点击减少人口", "fonts/arial.ttf", 20);
    hintLabel->setPosition(Vec2(visibleSize.width / 2, 180));
    hintLabel->setColor(Color3B::YELLOW);
    this->addChild(hintLabel, 1);

    // 已消耗人口显示
    auto assignedLabel = Label::createWithTTF("已消耗: 0", "fonts/arial.ttf", 20);
    assignedLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 190));
    assignedLabel->setColor(Color3B::YELLOW);
    assignedLabel->setTag(1000);
    this->addChild(assignedLabel, 1);

    updateUI();
}

void PopulationScene::updateUI()
{
    // 更新总人口显示
    int currentTotal = m_populationMgr->getTotalPopulation();
    m_totalLabel->setString(StringUtils::format("TOTLE: %d", currentTotal));

    // 更新可用人口
    int available = m_populationMgr->getAvailablePopulation();
    m_availableLabel->setString(StringUtils::format("totle: %d", available));

    // 更新已消耗人口
    int assigned = m_populationMgr->getAssignedPopulation();
    auto assignedLabel = (Label*)this->getChildByTag(1000);
    if (assignedLabel) {
        assignedLabel->setString(StringUtils::format("已消耗: %d", assigned));
    }

    // 更新人物数量显示
    m_warriorLabel->setString(StringUtils::format("%d",
        m_populationMgr->getCharacterCount(CharacterType::WARRIOR)));
    m_archerLabel->setString(StringUtils::format("%d",
        m_populationMgr->getCharacterCount(CharacterType::ARCHER)));
    m_mageLabel->setString(StringUtils::format("%d",
        m_populationMgr->getCharacterCount(CharacterType::MAGE)));
    m_healerLabel->setString(StringUtils::format("%d",
        m_populationMgr->getCharacterCount(CharacterType::HEALER)));

    // 如果可用人口为0，改变颜色
    if (available == 0) {
        m_availableLabel->setColor(Color3B::RED);
    }
    else {
        m_availableLabel->setColor(Color3B::WHITE);
    }
}

void PopulationScene::onCharacterClicked(CharacterType type, int count)
{
    if (m_populationMgr->isValidAssignment(type, count)) {
        m_populationMgr->assignCharacter(type, count);
        updateUI();

        // 播放点击动画
        Sprite* clickedSprite = nullptr;
        switch (type) {
        case CharacterType::WARRIOR: clickedSprite = m_warriorSprite; break;
        case CharacterType::ARCHER: clickedSprite = m_archerSprite; break;
        case CharacterType::MAGE: clickedSprite = m_mageSprite; break;
        case CharacterType::HEALER: clickedSprite = m_healerSprite; break;
        }

        if (clickedSprite) {
            clickedSprite->runAction(Sequence::create(
                ScaleTo::create(0.1f, 1.2f),
                ScaleTo::create(0.1f, 1.0f),
                nullptr
            ));
        }

        // 播放音效
        AudioEngine::play2d("click.mp3", false, 0.5f);

        // 显示增加提示
        showFeedback(type, "+1", Color3B::GREEN);
    }
    else {
        // 播放错误音效
        AudioEngine::play2d("error.mp3", false, 0.5f);

        // 显示错误信息
        showFeedback(type, "人口不足！", Color3B::RED);
    }
}

void PopulationScene::onCharacterRightClicked(CharacterType type, int count)
{
    if (m_populationMgr->isValidUnassignment(type, count)) {
        m_populationMgr->unassignCharacter(type, count);
        updateUI();

        // 播放点击动画
        Sprite* clickedSprite = nullptr;
        switch (type) {
        case CharacterType::WARRIOR: clickedSprite = m_warriorSprite; break;
        case CharacterType::ARCHER: clickedSprite = m_archerSprite; break;
        case CharacterType::MAGE: clickedSprite = m_mageSprite; break;
        case CharacterType::HEALER: clickedSprite = m_healerSprite; break;
        }

        if (clickedSprite) {
            clickedSprite->runAction(Sequence::create(
                ScaleTo::create(0.1f, 0.8f),
                ScaleTo::create(0.1f, 1.0f),
                nullptr
            ));
        }

        // 播放音效
        AudioEngine::play2d("click.mp3", false, 0.5f);

        // 显示减少提示
        showFeedback(type, "-1", Color3B::YELLOW);
    }
    else {
        // 播放错误音效
        AudioEngine::play2d("error.mp3", false, 0.5f);

        // 显示错误信息
        showFeedback(type, "数量为0！", Color3B::RED);
    }
}

void PopulationScene::showFeedback(CharacterType type, const std::string& message, Color3B color)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float spacingX = visibleSize.width / 5;
    float startY = visibleSize.height * 0.6f;

    // 根据人物类型确定位置
    float x = spacingX;
    switch (type) {
    case CharacterType::WARRIOR: x = spacingX; break;
    case CharacterType::ARCHER:  x = spacingX * 2; break;
    case CharacterType::MAGE:    x = spacingX * 3; break;
    case CharacterType::HEALER:  x = spacingX * 4; break;
    }

    auto feedbackLabel = Label::createWithTTF(message, "fonts/arial.ttf", 18);
    feedbackLabel->setPosition(Vec2(x, startY - 130));
    feedbackLabel->setColor(color);
    this->addChild(feedbackLabel, 10);

    // 淡出动画
    feedbackLabel->runAction(Sequence::create(
        DelayTime::create(1.0f),
        FadeOut::create(0.5f),
        CallFunc::create([feedbackLabel]() {
            feedbackLabel->removeFromParent();
            }),
        nullptr
    ));
}

void PopulationScene::onFightClicked(Ref* sender)
{
    if (m_populationMgr->isReadyForBattle()) {
        // 保存分配
        m_populationMgr->saveAssignment();

        // 播放成功音效
        AudioEngine::play2d("success.mp3", false, 0.7f);

        // 显示成功信息
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto successLabel = Label::createWithTTF("分配完成，开始战斗！", "fonts/arial.ttf", 30);
        successLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        successLabel->setColor(Color3B::GREEN);
        successLabel->setScale(0.1f);
        this->addChild(successLabel, 10);

        successLabel->runAction(Sequence::create(
            ScaleTo::create(0.3f, 1.2f),
            ScaleTo::create(0.2f, 1.0f),
            DelayTime::create(1.0f),
            FadeOut::create(0.5f),
            CallFunc::create([successLabel]() {
                successLabel->removeFromParent();
                }),
            nullptr
        ));

        CCLOG("Population assignment saved. Total assigned: %d",
            m_populationMgr->getAssignedPopulation());
    }
    else {
        // 播放错误音效
        AudioEngine::play2d("error.mp3", false, 0.5f);

        // 显示错误信息
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto errorLabel = Label::createWithTTF("请至少分配一个人口！", "fonts/arial.ttf", 24);
        errorLabel->setPosition(Vec2(visibleSize.width / 2, 200));
        errorLabel->setColor(Color3B::RED);
        this->addChild(errorLabel, 10);

        errorLabel->runAction(Sequence::create(
            DelayTime::create(1.5f),
            FadeOut::create(0.5f),
            CallFunc::create([errorLabel]() {
                errorLabel->removeFromParent();
                }),
            nullptr
        ));
    }
}

void PopulationScene::onBackClicked(Ref* sender)
{
    // 播放点击音效
    AudioEngine::play2d("click.mp3", false, 0.5f);

    // 返回上一个场景
    Director::getInstance()->popScene();
}
