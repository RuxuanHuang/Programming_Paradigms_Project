#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
USING_NS_CC;
class Building : public Node
{
public:
    static Building* create(const std::string& buildingFile,
        const std::string turfFile = "grass.png",
        float buildingScale = 0.7f);


    virtual ~Building() = default;

    bool isDragging() const { return _isDragging; }
    bool isSelected() const { return _isSelected; }
    cocos2d::Sprite* getTurf() const { return _turf; }
    cocos2d::Sprite* getBuildingSprite() const { return _buildingSprite; }

    void setBuildingScale(float scale);
    void setTurfScale(float scale);

    void snapToTile(Node* mapNode, int buildingTileW, int buildingTileH);

    bool isCenterInsideMap(Node* mapNode) const;
    void drawDebugMapRange(Node* mapNode);
    void setBuildingTileSize(int tileWidthCount, int tileHeightCount);
    bool isClickingInTurf(Node* mapNode, cocos2d::EventMouse* e);


    // ========== 标签相关方法 ==========
    void setBuildingName(const std::string& name);    // 设置建筑名称
    std::string getBuildingName() const { return _buildingName; }

    void setShowInfoLabel(bool show);                 // 控制是否显示标签
    void updateInfoLabel();                           // 更新标签内容

    // ========== 等级相关方法 ==========
    void setLevel(int level);                         // 设置等级
    int getLevel() const { return _level; }

    // ========== 新增：选择状态控制 ==========
    void setSelected(bool selected);

    // ========== 新增：操作栏相关方法 ==========
    void showActionBar();          // 显示操作栏
    static void hideActionBar();   // 隐藏操作栏（静态）

    // ========== 按钮回调方法（供子类覆盖）==========
    virtual void onInfoButtonClicked();
    virtual void onUpgradeButtonClicked();

protected:
    Vec2 _dragStartPos;


    Building();

    virtual bool init(const std::string& buildingFile,
        const std::string turfFile,
        float buildingScale);

    cocos2d::Sprite* _turf;
    cocos2d::Sprite* _buildingSprite;
    bool _isSelected;
    bool _isDragging;
    cocos2d::Vec2 _lastMousePos;
    float _buildingScaleRatio;

    cocos2d::DrawNode* _debugDrawNode;
    float _diamondWidthRatio;
    float _diamondHeightRatio;
    int _size;
    void onBuildingMouseDown(cocos2d::Event* event);
    void onBuildingMouseUp(cocos2d::Event* event);
    void onBuildingMouseMove(cocos2d::Event* event);
    void setupBuildingOnTurf();

    int _level;
    // ========== 新增：标签相关属性 ==========
    cocos2d::Label* _infoLabel;      // 信息标签
    std::string _buildingName;       // 建筑名称

    // ========== 标签控制方法 ==========
    virtual void createInfoLabel();  // 创建标签
    virtual void showInfoLabel();    // 显示标签
    virtual void hideInfoLabel();    // 隐藏标签
    

    
};

#endif // __BUILDING_H__