#ifndef __BUILDING_MANAGER_H__
#define __BUILDING_MANAGER_H__

#include "cocos2d.h"
#include "Building.h"
#include <map>
#include <vector>
#include <functional>

// 建筑配置结构体
struct BuildingConfig {
    std::string name;           // 建筑名称
    std::string type;           // 建筑类型
    std::string displayName;    // 显示名称
    std::string texturePath;    // 默认纹理路径
    int unlockLevel;            // 解锁所需的大本营等级
    int maxCount;               // 最大建造数量
    std::vector<int> numLimits; // 每个大本营等级对应的最大数量
    std::vector<int> upgradeLimits; // 每个大本营等级允许的最大建筑等级
    int width;                  // 网格宽度
    int height;                 // 网格高度
};

class BuildingManager {
public:
    // 单例模式
    static BuildingManager* getInstance();
    static void destroyInstance();

    // 初始化
    void initialize();

    // 建筑配置管理
    const BuildingConfig* getBuildingConfig(const std::string& type) const;
    const std::vector<BuildingConfig>& getAllBuildingConfigs() const;

    // 数量限制查询
    int getMaxCount(const std::string& buildingType, int townHallLevel) const;
    bool canBuildMore(const std::string& buildingType, int townHallLevel, int currentCount) const;

    // 建筑创建
    Building* createBuilding(const std::string& buildingType, const std::string& texturePath = "");
    Building* createBuildingFromCard(const std::string& cardName);

    // 类型转换
    std::string cardNameToType(const std::string& cardName) const;
    std::string typeToCardName(const std::string& type) const;

    // 解锁状态
    std::vector<std::string> getAvailableBuildings(int townHallLevel) const;
    bool isBuildingUnlocked(const std::string& buildingType, int townHallLevel) const;

    // 网格管理
    cocos2d::Vec2 findAvailablePosition(cocos2d::Node* mapNode,
        const cocos2d::Vector<Building*>& existingBuildings,
        Building* newBuilding) const;
    bool BuildingManager::isPositionAvailable(int gridX, int gridY,
        Building* building,
        const cocos2d::Vector<Building*>& existingBuildings) const;

    // 升级相关
    int getMaxLevel(const std::string& buildingType, int townHallLevel) const;
    bool canUpgradeBuilding(const std::string& buildingType, int currentLevel, int townHallLevel) const;



    void addTownhallLevel() { _townhallLevel++; }
    int getTownhallLevel() { return _townhallLevel; }
private:
    BuildingManager();
    ~BuildingManager();
    BuildingManager(const BuildingManager&) = delete;
    BuildingManager& operator=(const BuildingManager&) = delete;

    static BuildingManager* _instance;

    // 建筑配置数据
    std::vector<BuildingConfig> _buildingConfigs;
    std::map<std::string, BuildingConfig*> _typeToConfig;
    std::map<std::string, std::string> _cardNameToType; // 卡片名到类型映射
    std::map<std::string, std::string> _typeToCardName; // 类型到卡片名映射

    // 初始化建筑配置
    void initBuildingConfigs();

    // 初始化数量限制
    void initBuildingLimits();

    // 创建具体建筑
    Building* createCannon(const std::string& texturePath);
    Building* createArcherTower(const std::string& texturePath);
    Building* createWall(const std::string& texturePath);
    Building* createArmyCamp(const std::string& texturePath);
    Building* createGoldMine(const std::string& texturePath);
    Building* createElixirCollector(const std::string& texturePath);
    Building* createGoldStorage(const std::string& texturePath);
    Building* createElixirStorage(const std::string& texturePath);


    int _townhallLevel = 1;
    
};

#endif // __BUILDING_MANAGER_H__