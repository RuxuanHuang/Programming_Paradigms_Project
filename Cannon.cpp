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
   
    this->addChild(drawNode, -1); // 添加到加农炮底层，不遮挡炮台
    // -------------------------
    // 只有调用这个函数，才开启每秒扫描
    this->schedule(CC_SCHEDULE_SELECTOR(Cannon::scanForTargets), 1.0f);
}

// 替换 scanForTargets 为更稳健的版本（遍历活兵列表的拷贝）
void Cannon::scanForTargets(float dt) {
    auto scene = dynamic_cast<BattleScene1*>(Director::getInstance()->getRunningScene());
    if (!scene) return;

    // 如果当前目标已经无效，清空
    if (_currentTarget) {
        // 使用try-catch保护
        try {
            if (!_currentTarget->isAlive()) {
                _currentTarget = nullptr;
            }
            else {
                // 分步计算，更容易调试
                cocos2d::Vec2 targetPos;
                try {
                    targetPos = _currentTarget->getPosition();
                }
                catch (...) {
                    // 如果获取位置失败，说明对象无效
                    _currentTarget = nullptr;
                    return;
                }

                float distance = this->getPosition().distance(targetPos);
                if (distance > _attackingRange) {
                    _currentTarget = nullptr;
                }
            }
        }
        catch (...) {
            // 捕获任何异常，重置目标
            _currentTarget = nullptr;
        }
    }

    // 复制一份活兵列表，避免在迭代过程中原 vector 被修改导致迭代器失效
    auto allSoldiers = scene->getActiveSoldiers(); // 复制（要求 getActiveSoldiers 返回 vector<Soldier*> by value 或可拷贝）
    for (auto soldier : allSoldiers) {
        if (!soldier) continue;
        // 若该指针已从场景或父节点脱离，则跳过
        if (!soldier->isAlive() || soldier->getParent() == nullptr) continue;
        if (this->getPosition().distance(soldier->getPosition()) <= _attackingRange) {
            _currentTarget = soldier;
            break;
        }
    }

    // 攻击目标（若有）
    if (_currentTarget) {
        // 再做一次安全检查
        if (_currentTarget->getParent() == nullptr || !_currentTarget->isAlive()) {
            _currentTarget = nullptr;
            return;
        }
        // 对目标造成伤害（注意：reduceHP 可能会触发 onDeath -> enqueueDestruction，但不会立即删除）
        _currentTarget->reduceHP(_attackingPerSecond);
    }
}