#ifndef __RESOURCE_SYSTEM_H__
#define __RESOURCE_SYSTEM_H__

#include "cocos2d.h"

USING_NS_CC;

// 资源类型枚举
enum class ResourceType
{
    GOLD = 0,    // 金币
    CRYSTAL = 1  // 水晶
};

// 资源基类
class Resource
{
public:
    Resource(const std::string& name, int initialAmount);
    virtual ~Resource() {}

    // 通用接口
    virtual bool add(int amount);             // 增加资源
    virtual bool subtract(int amount);        // 减少资源
    virtual bool canAfford(int cost) const;    // 检查是否足够支付
    virtual int getAmount() const { return m_amount; }
    virtual std::string getName() const { return m_name; }

    // 获取/设置最大限额
    virtual int getMaxLimit() const { return m_maxLimit; }
    virtual void setMaxLimit(int limit) { m_maxLimit = limit; }

    // 重置资源
    virtual void reset(int amount = 0) { m_amount = amount; }

    // 检查是否达到上限
    virtual bool isFull() const { return m_amount >= m_maxLimit; }

protected:
    std::string m_name;      // 资源名称
    int m_amount;           // 当前数量
    int m_maxLimit;         // 最大限制（0表示无限制）
};

// 金币子类
class Gold : public Resource
{
public:
    Gold(int initialAmount = 100);
    virtual ~Gold() {}

    // 金币特有的方法
    void earnFromBattle(int reward);           // 战斗获得
    bool spendForItem(int cost);               // 购买物品
    bool spendForUpgrade(int cost);            // 升级花费
};

// 水晶子类
class Crystal : public Resource
{
public:
    Crystal(int initialAmount = 50);
    virtual ~Crystal() {}

    // 水晶特有的方法
    void earnFromQuest(int reward);            // 任务获得
    bool spendForPremiumItem(int cost);        // 购买高级物品
    bool spendForInstant(int cost);            // 立即完成花费
};

// 资源管理器（单例模式）
class ResourceManager
{
public:
    static ResourceManager* getInstance();
    static void destroyInstance();

    // 获取资源对象
    Gold* getGold() { return m_gold; }
    Crystal* getCrystal() { return m_crystal; }
    Resource* getResource(ResourceType type);

    // 资源操作
    bool canAffordPurchase(int goldCost, int crystalCost) const;
    bool makePurchase(int goldCost, int crystalCost, bool allowZero = true);
    void earnResources(int goldAmount, int crystalAmount);

    // 保存/加载
    void saveResources();
    void loadResources();

    // 重置
    void resetAllResources(int goldAmount = 100, int crystalAmount = 50);

    // 获取字符串表示
    std::string getGoldString() const;
    std::string getCrystalString() const;

    // 检查购买是否得零
    bool willPurchaseLeaveZero(int goldCost, int crystalCost) const;

private:
    ResourceManager();
    ~ResourceManager();
    static ResourceManager* s_instance;

    Gold* m_gold;
    Crystal* m_crystal;

    // 禁止拷贝
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
};

#endif // __RESOURCE_SYSTEM_H__