#include "Building.h"
#include "AudioEngine.h"

class ResourceCollector : public Building {
public:
    virtual bool init(const std::string& buildingFile, const std::string turfFile = "grass.png", float buildingScale = 0.8) override;

    // 每帧或每秒调用的生产逻辑
    virtual void updateProduction(float dt);

    // 收集行为
    virtual void collectResource();
    virtual void upgrade() override;

    // 重写Building基类的鼠标事件方法
    virtual void onBuildingMouseDown(Event* event) override;
    virtual void onBuildingMouseUp(Event* event) override;
    virtual void onBuildingMouseMove(Event* event) override;

    // 判断是否可收集
    bool isCollectable() const {
        return _isHomeTown&&_currentStored > 0 && _collectIcon && _collectIcon->isVisible();
    }
    void tryCollectResource();

protected:
    // 核心属性
    float _productionPerHour;  // 每小时产量
    float _maxCapacity;       // 最大容量
    float _currentStored;     // 当前已产出未收集的数量
    float _collectThreshold;   // 弹出“收集标识”的阈值百分比
    std::vector<int>storageCapacityList;
    // UI 元素
    cocos2d::Sprite* _collectIcon; // 收集图标

    // 虚函数供子类重写，以执行具体的资源加算（比如增加金币或木材）
    virtual void onCollected(float amount) = 0;

    // 初始化图标位置
    void setupCollectIcon(const std::string& iconFile);
    void updateIconVisibility();



    // 鼠标状态
    bool _mouseStartedOnCollectable = false;
    bool _shouldCollect = false;
    cocos2d::Vec2 _mouseStartPos;
    float _clickThreshold = 10.0f; // 点击判定阈值

    std::vector<std::pair<std::string, std::string>> ResourceCollector::getSpecificInfoItems()override;
};



class GoldMine : public ResourceCollector {
public:
    static GoldMine* create(const std::string& buildingFile, bool isHomeTown = true, const std::string turfFile = "grass.png", float buildingScale = 0.8);

    virtual bool init(const std::string& buildingFile, const std::string turfFile, float buildingScale) override;

protected:
    virtual void onCollected(float amount) override;
};

class ElixirCollector : public ResourceCollector {
public:
    static ElixirCollector* create(const std::string& buildingFile, bool isHomeTown = true, const std::string turfFile = "grass.png", float buildingScale = 0.8);

    virtual bool init(const std::string& buildingFile, const std::string turfFile, float buildingScale) override;

protected:
    virtual void onCollected(float amount) override;
};