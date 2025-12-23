#include "Building.h"
#include "ui/UILoadingBar.h"

USING_NS_CC;

class ResourceStorageBuilding : public Building
{
public:
    static ResourceStorageBuilding* create(const std::string& buildingFile,
        bool isHownTown = true,
        const std::string turfFile = "grass.png",
        float buildingScale = 0.8f);
    virtual ~ResourceStorageBuilding() = default;

    // ========== 资源存储相关方法 ==========
    void setMaxStorage(int maxStorage) { _maxStorage = maxStorage; }
    int getMaxStorage() const { return _maxStorage; }

    void setCurrentStorage(int storage) { _currentStorage = storage; }
    int getCurrentStorage() const { return _currentStorage; }

    virtual void upgrade() override;

protected:
    ResourceStorageBuilding();

    // 保护初始化方法
    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale);

    // ========== 存储容量相关 ==========
    int _maxStorage;           // 最大存储容量
    int _currentStorage;       // 当前存储量
    std::vector<int>storageCapacityList;

};

class GoldStorage : public ResourceStorageBuilding
{
public:
    static GoldStorage* create(const std::string& buildingFile = "Gold_Storage/Gold_Storage1.png",
        bool isHownTown = true,
        const std::string turfFile = "grass.png",
        float buildingScale = 0.7f);

    virtual ~GoldStorage() = default;

protected:

    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale) override;
};

class ElixirStorage : public ResourceStorageBuilding
{
public:
    static ElixirStorage* create(const std::string& buildingFile = "Elixir_Storage/Elixir_Storage1.png",
        bool isHownTown = true,
        const std::string turfFile = "grass.png",
        float buildingScale = 0.7f);



protected:

    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale) override;
};