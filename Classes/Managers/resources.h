#ifndef __RESOURCE_SYSTEM_H__
#define __RESOURCE_SYSTEM_H__

#include "cocos2d.h"

USING_NS_CC;

// 资源类型枚举
enum class ResourceType
{
    GOLD ,    // 金币
    ELIXIR  // 圣水
};

// 资源基类
class Resource
{
public:
    Resource(const std::string& name, int initialAmount=1000);
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
    Gold(int initialAmount = 500);
    virtual ~Gold() {}

    // 金币特有的方法
    void earnFromBattle(int reward);           // 战斗获得
    bool spendForItem(int cost);               // 购买物品
    bool spendForUpgrade(int cost);            // 升级花费
};

// 圣水子类
class Elixir : public Resource
{
public:
    Elixir(int initialAmount = 500);
    virtual ~Elixir() {}

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
    Elixir* getElixir() { return m_elixir; }
    Resource* getResource(ResourceType type);

    // 单个资源购买检查
    bool canAffordGold(int goldCost) const;
    bool canAffordElixir(int elixirCost) const;

    // 单个资源购买
    bool makeGoldPurchase(int goldCost, bool allowZero = true);
    bool makeElixirPurchase(int elixirCost, bool allowZero = true);

    // 获得资源（三种方式）
    void earnResources(int goldAmount, int elixirAmount);  // 同时获得两种
    void earnGold(int amount);                             // 只获得金币
    void earnElixir(int amount);                           // 只获得圣水

    // 保存/加载
    void saveResources();
    void loadResources();

    // 重置
    void resetAllResources(int goldAmount = 100, int elixirlAmount = 50);

    // 获取字符串表示
    int getGoldAmount() const;
    int getElixirAmount() const;

private:
    ResourceManager();
    ~ResourceManager();
    static ResourceManager* s_instance;

    Gold* m_gold;
    Elixir* m_elixir;


    
};

#endif // __RESOURCE_SYSTEM_H__