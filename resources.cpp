#include "resources.h"

// 静态成员初始化
ResourceManager* ResourceManager::s_instance = nullptr;

// ============ Resource 基类实现 ============
Resource::Resource(const std::string& name, int initialAmount)
    : m_name(name)
    , m_amount(initialAmount)
    , m_maxLimit(999999)  // 默认最大值
{
    CCLOG("Resource %s created with amount: %d", name.c_str(), initialAmount);
}

bool Resource::add(int amount)
{
    if (amount <= 0) {
        CCLOG("Error: Cannot add negative or zero amount to %s", m_name.c_str());
        return false;
    }

    // 检查是否会超过上限
    if (m_maxLimit > 0 && (m_amount + amount) > m_maxLimit) {
        CCLOG("Warning: Adding %d to %s would exceed max limit %d",
            amount, m_name.c_str(), m_maxLimit);
        m_amount = m_maxLimit;
        return true;  // 还是返回true，只是达到上限
    }

    m_amount += amount;
    CCLOG("%s increased by %d, now: %d", m_name.c_str(), amount, m_amount);
    return true;
}

bool Resource::subtract(int amount)
{
    if (amount <= 0) {
        CCLOG("Error: Cannot subtract negative or zero amount from %s", m_name.c_str());
        return false;
    }

    if (m_amount < amount) {
        CCLOG("Error: Insufficient %s. Need: %d, Have: %d",
            m_name.c_str(), amount, m_amount);
        return false;
    }

    m_amount -= amount;
    CCLOG("%s decreased by %d, now: %d", m_name.c_str(), amount, m_amount);
    return true;
}

bool Resource::canAfford(int cost) const
{
    if (cost < 0) {
        CCLOG("Error: Cost cannot be negative");
        return false;
    }

    bool canAfford = (m_amount >= cost);
    if (!canAfford) {
        CCLOG("Cannot afford %d %s, need %d more",
            cost, m_name.c_str(), cost - m_amount);
    }
    return canAfford;
}

// ============ Gold 子类实现 ============
Gold::Gold(int initialAmount)
    : Resource("Gold", initialAmount)
{
    setMaxLimit(9999999);  // 金币上限较高
}

void Gold::earnFromBattle(int reward)
{
    if (reward <= 0) {
        CCLOG("Battle reward must be positive");
        return;
    }

    // 战斗获得有额外奖励
    int actualReward = reward;
    if (!isFull()) {
        actualReward = reward;  // 这里可以添加额外逻辑
        add(actualReward);
    }
}

bool Gold::spendForItem(int cost)
{
    if (cost <= 0) {
        CCLOG("Item cost must be positive");
        return false;
    }

    return subtract(cost);
}

bool Gold::spendForUpgrade(int cost)
{
    if (cost <= 0) {
        CCLOG("Upgrade cost must be positive");
        return false;
    }

    // 升级花费可能有额外检查
    if (getAmount() - cost < 100) {  // 确保升级后至少保留100金币
        CCLOG("Cannot upgrade, would leave less than 100 gold");
        return false;
    }

    return subtract(cost);
}

// ============ Crystal 子类实现 ============
Crystal::Crystal(int initialAmount)
    : Resource("Crystal", initialAmount)
{
    setMaxLimit(99999);  // 水晶上限较低
}

void Crystal::earnFromQuest(int reward)
{
    if (reward <= 0) {
        CCLOG("Quest reward must be positive");
        return;
    }

    // 任务获得可能有额外处理
    int actualReward = reward;
    if (!isFull()) {
        actualReward = reward;
        add(actualReward);
    }
}

bool Crystal::spendForPremiumItem(int cost)
{
    if (cost <= 0) {
        CCLOG("Premium item cost must be positive");
        return false;
    }

    return subtract(cost);
}

bool Crystal::spendForInstant(int cost)
{
    if (cost <= 0) {
        CCLOG("Instant completion cost must be positive");
        return false;
    }

    // 立即完成花费有特殊检查
    if (getAmount() < cost * 2) {  // 确保有足够的水晶储备
        CCLOG("Warning: Low crystal reserve for instant completion");
    }

    return subtract(cost);
}

// ============ ResourceManager 实现 ============
ResourceManager::ResourceManager()
{
    m_gold = new Gold(100);
    m_crystal = new Crystal(50);
    CCLOG("ResourceManager created");
}

ResourceManager::~ResourceManager()
{
    delete m_gold;
    delete m_crystal;
    CCLOG("ResourceManager destroyed");
}

ResourceManager* ResourceManager::getInstance()
{
    if (!s_instance) {
        s_instance = new ResourceManager();
    }
    return s_instance;
}

void ResourceManager::destroyInstance()
{
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

Resource* ResourceManager::getResource(ResourceType type)
{
    switch (type) {
    case ResourceType::GOLD:
        return m_gold;
    case ResourceType::CRYSTAL:
        return m_crystal;
    default:
        return nullptr;
    }
}

bool ResourceManager::canAffordPurchase(int goldCost, int crystalCost) const
{
    bool canAffordGold = m_gold->canAfford(goldCost);
    bool canAffordCrystal = m_crystal->canAfford(crystalCost);

    if (!canAffordGold || !canAffordCrystal) {
        CCLOG("Cannot afford purchase. Gold: %s, Crystal: %s",
            canAffordGold ? "OK" : "Insufficient",
            canAffordCrystal ? "OK" : "Insufficient");
        return false;
    }

    return true;
}

bool ResourceManager::makePurchase(int goldCost, int crystalCost, bool allowZero)
{
    // 检查是否足够
    if (!canAffordPurchase(goldCost, crystalCost)) {
        return false;
    }

    // 检查购买后是否得零
    if (!allowZero && willPurchaseLeaveZero(goldCost, crystalCost)) {
        CCLOG("Purchase would leave zero resources, not allowed");
        return false;
    }

    // 执行购买
    bool goldSuccess = m_gold->subtract(goldCost);
    bool crystalSuccess = m_crystal->subtract(crystalCost);

    if (goldSuccess && crystalSuccess) {
        CCLOG("Purchase successful. Gold: -%d, Crystal: -%d", goldCost, crystalCost);
        return true;
    }
    else {
        // 如果有一个失败，回滚
        if (goldSuccess) m_gold->add(goldCost);
        if (crystalSuccess) m_crystal->add(crystalCost);
        CCLOG("Purchase failed and rolled back");
        return false;
    }
}

bool ResourceManager::willPurchaseLeaveZero(int goldCost, int crystalCost) const
{
    int goldAfter = m_gold->getAmount() - goldCost;
    int crystalAfter = m_crystal->getAmount() - crystalCost;

    return (goldAfter == 0 || crystalAfter == 0);
}

void ResourceManager::earnResources(int goldAmount, int crystalAmount)
{
    if (goldAmount > 0) {
        m_gold->add(goldAmount);
    }

    if (crystalAmount > 0) {
        m_crystal->add(crystalAmount);
    }
}

void ResourceManager::saveResources()
{
    // 这里可以保存到UserDefault或文件
    UserDefault* userDefault = UserDefault::getInstance();
    userDefault->setIntegerForKey("player_gold", m_gold->getAmount());
    userDefault->setIntegerForKey("player_crystal", m_crystal->getAmount());
    userDefault->flush();
    CCLOG("Resources saved");
}

void ResourceManager::loadResources()
{
    UserDefault* userDefault = UserDefault::getInstance();
    int gold = userDefault->getIntegerForKey("player_gold", 100);
    int crystal = userDefault->getIntegerForKey("player_crystal", 50);

    m_gold->reset(gold);
    m_crystal->reset(crystal);
    CCLOG("Resources loaded: Gold=%d, Crystal=%d", gold, crystal);
}

void ResourceManager::resetAllResources(int goldAmount, int crystalAmount)
{
    m_gold->reset(goldAmount);
    m_crystal->reset(crystalAmount);
    CCLOG("Resources reset: Gold=%d, Crystal=%d", goldAmount, crystalAmount);
}

std::string ResourceManager::getGoldString() const
{
    return StringUtils::format("%d", m_gold->getAmount());
}

std::string ResourceManager::getCrystalString() const
{
    return StringUtils::format("%d", m_crystal->getAmount());
}