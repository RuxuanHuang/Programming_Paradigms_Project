#include "BuildingManager.h"
#include "TownHall.h"
#include "BuildersHut.h"
#include "Cannon.h"
#include "Archer_Tower.h"
#include "Wall.h"
#include "Army_Camp.h"
#include "ResourceCollector.h"
#include "ResourceStorageBuilding.h"

USING_NS_CC;

// 静态变量初始化
BuildingManager* BuildingManager::_instance = nullptr;

BuildingManager* BuildingManager::getInstance() {
    if (!_instance) {
        _instance = new BuildingManager();
        _instance->initialize();
    }
    return _instance;
}

void BuildingManager::destroyInstance() {
    CC_SAFE_DELETE(_instance);
}

BuildingManager::BuildingManager() {
}

BuildingManager::~BuildingManager() {
    // 清理资源
    _typeToConfig.clear();
    _cardNameToType.clear();
    _typeToCardName.clear();
}

void BuildingManager::initialize() {
    initBuildingConfigs();
    initBuildingLimits();
}

void BuildingManager::initBuildingConfigs() {
    _buildingConfigs.clear();
    _typeToConfig.clear();
    _cardNameToType.clear();
    _typeToCardName.clear();

    // 定义所有建筑配置
    std::vector<BuildingConfig> configs = {
       
        //大本营
        {"townhall", "TownHall", "Town Hall", "Town_hall/Town_hall1.png",
        1, 1, {1,1,1},{2,3,4}, 4, 4},
        // 防御建筑
        {"cannon", "Cannon", "Cannon", "Cannon/Cannon1.png",
        1, 2, {1,2,2},{2,3,4}, 3, 3},

        {"archertower", "ArcherTower", "Archer Tower", "Archer_Tower/Archer_Tower1.png",
        2, 1, {0,1,1},{0,2,3}, 3, 3},

        // 城墙
        {"wall", "Wall", "Wall", "Wall/Wall1.png",
        1, 75, {25,50,75}, {1,2,3},1, 1},

        // 资源建筑
        {"goldmine", "GoldMine", "Gold Mine", "Gold_Mine/Gold_Mine1.png",
            1, 3, {1,2,3},{2,4,6}, 3, 3},

        {"elixircollector", "ElixirCollector", "Elixir Collector", "Elixir_Collector/Elixir_Collector1.png",
            1, 3, {1,2,3}, {2,4,6},3, 3},

        {"goldstorage", "GoldStorage", "Gold Storage", "Gold_Storage/Gold_Storage1.png",
            1, 2, {1,1,2},{1,3,6}, 3, 3},

        {"elixirstorage", "ElixirStorage", "Elixir Storage", "Elixir_Storage/Elixir_Storage1.png",
            1, 2, {1,1,2},{1,3,6}, 3, 3},

        // 军队建筑
        {"armycamp", "ArmyCamp", "Army Camp", "Army_Camp/Army_Camp1.png",
        1, 2, {1,1,2},{1,2,3} ,4, 4},
    };

    // 存储配置并建立索引
    for (auto& config : configs) {
        _buildingConfigs.push_back(config);
        _typeToConfig[config.type] = &_buildingConfigs.back();

        // 建立卡片名映射
        _cardNameToType[config.displayName] = config.type;
        _typeToCardName[config.type] = config.displayName;
    }
}

void BuildingManager::initBuildingLimits() {
    // 不再需要在Building类中静态存储，现在由Manager统一管理
}

const BuildingConfig* BuildingManager::getBuildingConfig(const std::string& type) const {
    auto it = _typeToConfig.find(type);
    return (it != _typeToConfig.end()) ? it->second : nullptr;
}

const std::vector<BuildingConfig>& BuildingManager::getAllBuildingConfigs() const {
    return _buildingConfigs;
}

int BuildingManager::getMaxCount(const std::string& buildingType, int townHallLevel) const {
    const BuildingConfig* config = getBuildingConfig(buildingType);
    if (!config) return 0;

    if (townHallLevel < 1) townHallLevel = 1;
    if (townHallLevel > 3) townHallLevel = 3;

    // 返回该大本营等级对应的最大数量
    return config->numLimits[townHallLevel - 1];
}

int BuildingManager::getMaxLevel(const std::string& buildingType, int townHallLevel) const {
    const BuildingConfig* config = getBuildingConfig(buildingType);
    if (!config) return 0;

    if (townHallLevel < 1) townHallLevel = 1;
    if (townHallLevel > 3) townHallLevel = 3;

    // 返回该大本营等级对应的最大等级
    return config->upgradeLimits[townHallLevel - 1];
}



bool BuildingManager::canBuildMore(const std::string& buildingType, int townHallLevel, int currentCount) const {
    int maxCount = getMaxCount(buildingType, townHallLevel);
    if (maxCount <= 0) return false;

    return currentCount < maxCount;
}

Building* BuildingManager::createBuilding(const std::string& buildingType, const std::string& texturePath) {
    std::string actualTexturePath = texturePath;

    // 如果没指定纹理，使用默认纹理
    const BuildingConfig* config = getBuildingConfig(buildingType);
    if (texturePath.empty() && config) {
        actualTexturePath = config->texturePath;
    }

    if (buildingType == "Cannon") {
        return createCannon(actualTexturePath);
    }
    else if (buildingType == "ArcherTower") {
        return createArcherTower(actualTexturePath);
    }
    else if (buildingType == "Wall") {
        return createWall(actualTexturePath);
    }
    else if (buildingType == "ArmyCamp") {
        return createArmyCamp(actualTexturePath);
    }
    else if (buildingType == "GoldMine") {
        return createGoldMine(actualTexturePath);
    }
    else if (buildingType == "ElixirCollector") {
        return createElixirCollector(actualTexturePath);
    }
    else if (buildingType == "GoldStorage") {
        return createGoldStorage(actualTexturePath);
    }
    else if (buildingType == "ElixirStorage") {
        return createElixirStorage(actualTexturePath);
    }

    CCLOG("BuildingManager: Unknown building type: %s", buildingType.c_str());
    return nullptr;
}

Building* BuildingManager::createBuildingFromCard(const std::string& cardName) {
    std::string buildingType = cardNameToType(cardName);
    if (buildingType.empty()) {
        CCLOG("BuildingManager: Unknown card name: %s", cardName.c_str());
        return nullptr;
    }

    return createBuilding(buildingType);
}

std::string BuildingManager::cardNameToType(const std::string& cardName) const {
    auto it = _cardNameToType.find(cardName);
    return (it != _cardNameToType.end()) ? it->second : "";
}

std::string BuildingManager::typeToCardName(const std::string& type) const {
    auto it = _typeToCardName.find(type);
    return (it != _typeToCardName.end()) ? it->second : "";
}

std::vector<std::string> BuildingManager::getAvailableBuildings(int townHallLevel) const {
    std::vector<std::string> available;

    for (const auto& config : _buildingConfigs) {
        if (isBuildingUnlocked(config.type, townHallLevel)) {
            available.push_back(config.type);
        }
    }

    return available;
}

bool BuildingManager::isBuildingUnlocked(const std::string& buildingType, int townHallLevel) const {
    const BuildingConfig* config = getBuildingConfig(buildingType);
    if (!config) return false;

    return townHallLevel >= config->unlockLevel;
}


Building* BuildingManager::createCannon(const std::string& texturePath) {
    auto cannon = Cannon::create(texturePath);
    if (cannon) {
        cannon->setBuildingType("Cannon");
        
    }
    return cannon;
}

Building* BuildingManager::createArcherTower(const std::string& texturePath) {
    auto archerTower = ArcherTower::create(texturePath);
    if (archerTower) {
        archerTower->setBuildingType("ArcherTower");
        
    }
    return archerTower;
}

Building* BuildingManager::createWall(const std::string& texturePath) {
    auto wall = Wall::create(texturePath);
    if (wall) {
        wall->setBuildingType("Wall");
        
    }
    return wall;
}

Building* BuildingManager::createArmyCamp(const std::string& texturePath) {
    auto armyCamp = ArmyCamp::create(texturePath);
    if (armyCamp) {
        armyCamp->setBuildingType("ArmyCamp");
        
    }
    return armyCamp;
}

Building* BuildingManager::createGoldMine(const std::string& texturePath) {
    auto goldMine = GoldMine::create(texturePath);
    if (goldMine) {
        goldMine->setBuildingType("GoldMine");
        
    }
    return goldMine;
}

Building* BuildingManager::createElixirCollector(const std::string& texturePath) {
    auto elixirCollector = ElixirCollector::create(texturePath);
    if (elixirCollector) {
        elixirCollector->setBuildingType("ElixirCollector");
        
    }
    return elixirCollector;
}

Building* BuildingManager::createGoldStorage(const std::string& texturePath) {
    auto goldStorage = GoldStorage::create(texturePath);
    if (goldStorage) {
        goldStorage->setBuildingType("GoldStorage");
        ResourceManager* resourceManager = ResourceManager::getInstance();
        resourceManager->updateGoldMaxLimit(goldStorage->getMaxStorage());
    }
    return goldStorage;
}

Building* BuildingManager::createElixirStorage(const std::string& texturePath) {
    auto elixirStorage = ElixirStorage::create(texturePath);
    if (elixirStorage) {
        elixirStorage->setBuildingType("ElixirStorage");
        ResourceManager* resourceManager = ResourceManager::getInstance();
        resourceManager->updateElixirMaxLimit(elixirStorage->getMaxStorage());
    }
    return elixirStorage;
}

Vec2 BuildingManager::findAvailablePosition(Node* mapNode,
    const cocos2d::Vector<Building*>& existingBuildings,
    Building* newBuilding) const {
    // 简单实现：从中心向外螺旋查找
    const int maxRadius = 20;

    for (int radius = 0; radius < maxRadius; radius++) {
        for (int x = -radius; x <= radius; x++) {
            for (int y = -radius; y <= radius; y++) {
                if (abs(x) == radius || abs(y) == radius) {
                    // 检查位置是否可用
                    int gridX = x + 10; // 从中心偏移
                    int gridY = y + 10;

                    if (isPositionAvailable(gridX, gridY, newBuilding, existingBuildings)) {
                        return Vec2(gridX, gridY);
                    }
                }
            }
        }
    }

    return Vec2(0, 0);
}


bool BuildingManager::isPositionAvailable(int gridX, int gridY,
     Building* building,
    const cocos2d::Vector<Building*>& existingBuildings) const {
    if (!building) return false;

    // 构建新建筑的网格矩形
    Rect newRect(gridX, gridY, building->getSize(), building->getSize());

    // 检查是否与其他建筑重叠
    for (const auto& existing : existingBuildings) {
        if (!existing) continue;

        Rect existingRect(existing->getGridX(), existing->getGridY(),
            existing->getSize(), existing->getSize());

        if (newRect.intersectsRect(existingRect)) {
            return false;
        }
    }

    return true;
}



bool BuildingManager::canUpgradeBuilding(const std::string& buildingType, int currentLevel, int townHallLevel) const {
    const BuildingConfig* config = getBuildingConfig(buildingType);
    int maxLevel = getMaxLevel(buildingType, townHallLevel);
    if (maxLevel <= 0) return false;

    return currentLevel < maxLevel;
}

