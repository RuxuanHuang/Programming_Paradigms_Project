#ifndef __BATTLESCENE1_H__
#define __BATTLESCENE1_H__

#include "cocos2d.h"
#include "Building.h" // 包含基类头文件
#include "ui/CocosGUI.h"
#include "Soldier.h"
#include <queue>
#include <unordered_map>
#include "Camp.h" // 添加这行：包含Camp场景的头文件
USING_NS_CC;
using namespace ui;


class BattleScene1 : public cocos2d::Scene
{
private:
    std::vector<Building*> _buildings;  // 存储所有建筑
    Building* _targetBuilding = nullptr; // 目标建筑（可以改为多个目标）

public:
    void visualizeBlockedTiles();
    // 添加这些公共方法
    void setTargetBuilding(Building* building) { _targetBuilding = building; }
    Building* getTargetBuilding() const { return _targetBuilding; }
    const std::vector<Building*>& getAllBuildings() const { return _buildings; }

public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    void menuCloseCallback(cocos2d::Ref* pSender);
    CREATE_FUNC(BattleScene1);
    void clearSelection();
    void selectBuilding(Building* building);

    void createPopulationButton(); // 新增：声明人口按钮创建方法
    virtual void onEnter() override;
    // 声明士兵创建函数
    void spawnSoldier(Soldier::Type type, cocos2d::Vec2 pos);

    // 新增：公有接口，用于添加士兵到销毁队列
    void addToDestroyQueue(Soldier* soldier) {
        _destroyQueue.push(soldier);
        CCLOG("[入队] 兵种：%d，当前队列长度：%d", (int)soldier->getType(), _destroyQueue.size());
    }
    // 新增：公有接口，返回是否正在销毁中
    static bool isDestroying() {
        return _isDestroying;
    }
    // 新增：声明处理销毁队列的函数
    void processDestroyQueue();


    std::queue<Soldier*> _attackQueue; // 攻击范围队列（替代原有销毁队列）
    float _attackRange = 500.0f; // 攻击范围半径（与原范围检测一致）
    Vec2 _attackTargetPos; // 攻击点坐标
    float _damagePerSec = 10.0f; // 每秒扣血量（可自定义）
    bool _isQueueProcessing = false; // 队列是否正在处理
    // 新增方法
    void addToAttackQueue(Soldier* soldier); // 加入攻击队列
    void processAttackQueue(); // 处理队列（启动下一个士兵受击）
    bool isInAttackRange(Soldier* soldier); // 判断是否进入攻击范围
    
private:
    // 新增：返回按钮
    cocos2d::ui::Button* _backButton = nullptr;

    // 新增：返回按钮创建方法
    void createBackButton();

    // 新增：返回按钮回调函数
    void backToCampCallback(cocos2d::Ref* pSender);

    // --- 场景内部变量 ---

    // 使用基类指针存储建筑对象
    Building* _building;

    // 地图拖动状态
    bool _isMapDragging;
    cocos2d::Vec2 _lastMapMousePos;

    // --- 地图/场景事件回调 ---
    void onMapMouseDown(cocos2d::Event* event);
    void onMapMouseUp(cocos2d::Event* event);
    void onMapMouseMove(cocos2d::Event* event);
    void onMouseScroll(cocos2d::Event* event);

    // 边界限制
    void limitMapPos(cocos2d::Sprite* sprite);

    Building* _selectedBuilding = nullptr;
    bool _mapMoved = false;

    // 新增：战斗状态
    bool _isBattleStarted;  // 战斗是否已开始

    // 新增：检查是否可以进入人口分配场景
    bool canEnterPopulationScene() const;

    Sprite* populationButton; // 新增：声明为类成员变量

    // 从人口分配场景加载的最大释放数量
    void loadMaxReleaseCountsFromPopulationManager();
    // 重置释放计数
    void resetSoldierReleaseCounts();

    // ========== 临时：各兵种等级（后续替换为队友接口） ==========
    int _infantryLevel = 1;    // 步兵等级
    int _archerLevel = 1;      // 弓箭手等级
    int _cavalryLevel = 1;     // 骑兵等级
    int _mageLevel = 1;        // 法师等级

    // 临时：获取兵种等级（后续替换为队友接口调用）
    int getTempSoldierLevel(Soldier::Type type);

    // 新增：静态队列，记录进入目标范围的士兵（按顺序）
    static std::queue<Soldier*> _destroyQueue;
    // 新增：标记当前是否有士兵在销毁倒计时中
    static bool _isDestroying;
    // 原触摸回调保留
    void initTouchListener();
    // 底部选择界面
    void initSoldierSelector();
    // 士兵队列处理（原逻辑保留）
    void processSoldierQueue();
    // 更新计数标签
    void updateCountLabel(Soldier::Type type);

    std::queue<Soldier*> _soldierQueue; // 士兵队列
    bool _isProcessingQueue = false;
    const float SOLDIER_SPEED = 200.0f;

    // 选择相关变量
    Soldier::Type _selectedType = Soldier::Type::INFANTRY;
    std::unordered_map<Soldier::Type, int> _soldierReleaseCounts;
    std::unordered_map<Soldier::Type, int> _maxReleaseCounts = {
        {Soldier::Type::INFANTRY, 10},
        {Soldier::Type::ARCHER, 8},
        {Soldier::Type::CAVALRY, 5},
        {Soldier::Type::MAGE, 3}
    };
    std::unordered_map<Soldier::Type, Label*> _countLabels;
    
};

#endif // __CAMP_H__