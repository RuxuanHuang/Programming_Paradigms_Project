#include "Archer_Tower.h"
#include "BattleScene1.h"
#include "Soldier.h"
USING_NS_CC;

ArcherTower* ArcherTower::create(const std::string& buildingFile,
    bool isHownTown,
    const std::string& turfFile,
    float buildingScale)
{
    ArcherTower* ret = new (std::nothrow) ArcherTower();
    ret->setIsHownTown(isHownTown);
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ArcherTower::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    // 1. 先调用父类的init
    if (!Building::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 2. 设置基本属性
    this->setBuildingName("Archer_Tower");
    this->setBuildingSize(3);
    this->setBuildingScale(0.9f);
    this->setMaxLevel(3);
    this->setCost(1000);


    int hpValues[] = { 380,420,460 };
    int upgradeCosts[] = { 2000,5000,0 };
    // 3. 设置等级信息
    for (int i = 1; i <= 3; i++) {
        _upgradeSprites[i] = levelInformation{ hpValues[i - 1], upgradeCosts[i - 1], "" };
        std::string spriteFile = StringUtils::format("Archer_Tower/Archer_Tower%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }

    _attackingPerSecond = 11;
    _attackingRange = 250.0f;
    attackingPerSecondList = { 11,15,19 };
    this->startCombatAI();
    return true;
}

void ArcherTower::upgrade()
{
    // 先调用基类的升级逻辑
    Building::upgrade();

    _attackingPerSecond = attackingPerSecondList[_level - 1];
}

void ArcherTower::startCombatAI() {
    // 可视化攻击范围 (淡淡的蓝色，区别于加农炮的红色)
    auto drawNode = DrawNode::create();
    drawNode->drawDot(Vec2::ZERO, _attackingRange, Color4F(0.0f, 0.5f, 1.0f, 0.1f));
    drawNode->drawCircle(Vec2::ZERO, _attackingRange, 0, 50, false, Color4F(0.0f, 0.5f, 1.0f, 0.3f));
    this->addChild(drawNode, -1);

    // 每秒扫描一次目标
    this->schedule(CC_SCHEDULE_SELECTOR(ArcherTower::scanForTargets), 1.0f);
}
void ArcherTower::scanForTargets(float dt) {
    auto scene = dynamic_cast<BattleScene1*>(Director::getInstance()->getRunningScene());
    if (!scene) return;

    // 1. 检查当前目标是否失效
    if (_currentTarget) {
        bool isOutOfRange = this->getPosition().distance(_currentTarget->getPosition()) > _attackingRange;
        if (!_currentTarget->isAlive() || isOutOfRange || _currentTarget->getParent() == nullptr) {
            _currentTarget = nullptr;
        }
    }

    // 2. 寻找新目标
    if (!_currentTarget) {
        auto& allSoldiers = scene->getActiveSoldiers();
        for (auto soldier : allSoldiers) {
            if (soldier && soldier->isAlive() && soldier->getParent()) {
                if (this->getPosition().distance(soldier->getPosition()) <= _attackingRange) {
                    _currentTarget = soldier;
                    break;
                }
            }
        }
    }

    // 3. 执行攻击
    if (_currentTarget) {
        CCLOG("[ArcherTower] 射击士兵！伤害: %.1f", _attackingPerSecond);
        _currentTarget->reduceHP(_attackingPerSecond);

        // 箭塔的小动画：缩放一下表示射箭
        this->runAction(Sequence::create(ScaleTo::create(0.1f, 0.95f), ScaleTo::create(0.1f, 0.9f), nullptr));
    }
}