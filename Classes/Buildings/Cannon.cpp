#include "Cannon.h"
#include "BattleScene1.h"
#include"Soldier.h"
USING_NS_CC;

Cannon* Cannon::create(const std::string& buildingFile,
    bool isHomeTown,
    const std::string& turfFile,
    float buildingScale)
{
    Cannon* ret = new (std::nothrow) Cannon();
    ret->setIsHownTown(isHomeTown);
    if (ret && ret->init(buildingFile, turfFile, buildingScale))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Cannon::init(const std::string& buildingFile,
    const std::string turfFile,
    float buildingScale)
{
    // 1. 先调用父类的init
    if (!Building::init(buildingFile, turfFile, buildingScale)) {
        return false;
    }

    // 2. 设置大本营基本属性
    this->setBuildingName("Cannon");
    this->setBuildingSize(3);
    this->setBuildingScale(0.9f);
    this->setMaxLevel(4);
    this->setCost(1000);

    int hpValues[] = { 300,360,420,500 };
    int upgradeCosts[] = { 1000,4000,16000,0 };
    // 3. 设置等级信息
    for (int i = 1; i <= 4; i++) {
        _upgradeSprites[i] = levelInformation{ hpValues[i - 1], upgradeCosts[i - 1], "" };
        std::string spriteFile = StringUtils::format("Cannon/Cannon%d.png", i);
        this->setUpgradeSprite(i, spriteFile);
    }

    _attackingPerSecond = 7;
    _attackingRange = 150.0f;
    attackingPerSecondList = { 7,10,13,17 };
    return true;
}

void Cannon::upgrade()
{
    // 先调用基类的升级逻辑
    Building::upgrade();

    _attackingPerSecond = attackingPerSecondList[_level - 1];
}
void Cannon::startCombatAI() {
    // --- 新增：可视化攻击范围 ---
    auto drawNode = DrawNode::create();
    // 参数：中心点、半径、填充颜色(透明)、边框粗细、边框颜色
    drawNode->drawDot(Vec2::ZERO, _attackingRange, Color4F(1.0f, 0.0f, 0.0f, 0.15f)); // 淡淡的红色圆
    drawNode->drawCircle(Vec2::ZERO, _attackingRange, 0, 50, false, Color4F(1.0f, 0.0f, 0.0f, 0.5f)); // 明显的边框

    this->addChild(drawNode, -1); // 添加到加农炮底层，不遮挡炮台
    // -------------------------
    // 只有调用这个函数，才开启每秒扫描
    this->schedule(CC_SCHEDULE_SELECTOR(Cannon::scanForTargets), 1.0f);
}
void Cannon::scanForTargets(float dt) {
    //  获取战斗场景实例
    auto scene = dynamic_cast<BattleScene1*>(Director::getInstance()->getRunningScene());
    if (!scene) return;

    //  获取场景中所有的士兵
    if (_currentTarget) {
        // 如果目标死了、被移除了、或者超出了范围，清除可视化并重置
        if (!_currentTarget->isAlive() ) {

            _currentTarget = nullptr;
        }
        else if (this->getPosition().distance(_currentTarget->getPosition()) > _attackingRange) {
            _currentTarget = nullptr;
        }
    }
    // --- 2. 寻找新目标 ---
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
    // --- 3. 扣血逻辑 ---
    if (_currentTarget) {
        if (_currentTarget->getParent() == nullptr || _currentTarget->getReferenceCount() <= 0) {
            _currentTarget = nullptr;
            return;
        }
        _currentTarget->reduceHP(_attackingPerSecond);
    }
}
