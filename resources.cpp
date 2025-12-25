#include "resources.h"

// 静态成员初始化
ResourceManager* ResourceManager::s_instance = nullptr;

// ============ Resource 基类实现 ============
Resource::Resource(const std::string& name, int initialAmount)
    : m_name(name)
    , m_amount(initialAmount)
    , m_maxLimit(1000)  // 默认最大值
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
    setMaxLimit(1000);  // 金币上限较高
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



    return subtract(cost);
}

// ============ Elixir 子类实现 ============
Elixir::Elixir(int initialAmount)
    : Resource("Elixir", initialAmount)
{
    setMaxLimit(1000);  // 圣水上限较低
}

void Elixir::earnFromQuest(int reward)
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

bool Elixir::spendForPremiumItem(int cost)
{
    if (cost <= 0) {
        CCLOG("Premium item cost must be positive");
        return false;
    }

    return subtract(cost);
}

bool Elixir::spendForInstant(int cost)
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
    m_gold = new Gold(500);
    m_elixir = new Elixir(500);  // 改为m_elixir
    CCLOG("ResourceManager created");
}

ResourceManager::~ResourceManager()
{
    delete m_gold;
    delete m_elixir;  // 改为m_elixir
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
    case ResourceType::ELIXIR:
        return m_elixir;
    default:
        return nullptr;
    }
}

bool ResourceManager::canAffordGold(int goldCost) const
{
    return m_gold->canAfford(goldCost);
}

bool ResourceManager::canAffordElixir(int elixirCost) const
{
    return m_elixir->canAfford(elixirCost);  // 改为m_elixir
}

bool ResourceManager::makeGoldPurchase(int goldCost, bool allowZero)
{
    if (!canAffordGold(goldCost)) {
        return false;
    }

    if (!allowZero && (m_gold->getAmount() - goldCost == 0)) {
        CCLOG("Gold purchase would leave zero resources, not allowed");
        return false;
    }

    return m_gold->subtract(goldCost);
}

bool ResourceManager::makeElixirPurchase(int elixirCost, bool allowZero)
{
    if (!canAffordElixir(elixirCost)) {
        return false;
    }

    if (!allowZero && (m_elixir->getAmount() - elixirCost == 0)) {
        CCLOG("Elixir purchase would leave zero resources, not allowed");
        return false;
    }

    return m_elixir->subtract(elixirCost);  // 改为m_elixir
}

void ResourceManager::earnGold(int amount)
{
    if (amount > 0) {
        m_gold->add(amount);
        CCLOG("Earned %d gold, total: %d", amount, m_gold->getAmount());
    }
    else if (amount < 0) {
        CCLOG("Warning: Trying to earn negative gold: %d", amount);
    }
}

void ResourceManager::earnElixir(int amount)
{
    if (amount > 0) {
        m_elixir->add(amount);  // 改为m_elixir
        CCLOG("Earned %d elixir, total: %d", amount, m_elixir->getAmount());  // 改为m_elixir
    }
    else if (amount < 0) {
        CCLOG("Warning: Trying to earn negative elixir: %d", amount);
    }
}

void ResourceManager::earnResources(int goldAmount, int elixirAmount)
{
    if (goldAmount > 0) {
        earnGold(goldAmount);
    }

    if (elixirAmount > 0) {
        earnElixir(elixirAmount);
    }

    if (goldAmount <= 0 && elixirAmount <= 0) {
        CCLOG("Warning: earnResources called with non-positive amounts: gold=%d, elixir=%d",
            goldAmount, elixirAmount);
    }
}
void ResourceManager::saveResources()
{
    // 这里可以保存到UserDefault或文件
    UserDefault* userDefault = UserDefault::getInstance();
    userDefault->setIntegerForKey("player_gold", m_gold->getAmount());
    userDefault->setIntegerForKey("player_crystal", m_elixir->getAmount());
    userDefault->flush();
    CCLOG("Resources saved");
}

void ResourceManager::loadResources()
{
    UserDefault* userDefault = UserDefault::getInstance();
    int gold = userDefault->getIntegerForKey("player_gold", 500);
    int elixir = userDefault->getIntegerForKey("player_elixir", 500);

    m_gold->reset(gold);
    m_elixir->reset(elixir);

}

void ResourceManager::resetAllResources(int goldAmount, int elixirAmount)
{
    m_gold->reset(goldAmount);
    m_elixir->reset(elixirAmount);
    CCLOG("Resources reset: Gold=%d, Crystal=%d", goldAmount, elixirAmount);
}


int ResourceManager::getGoldAmount() const {
    return m_gold->getAmount();
}

int ResourceManager::getElixirAmount() const {
    return m_elixir->getAmount();
}
