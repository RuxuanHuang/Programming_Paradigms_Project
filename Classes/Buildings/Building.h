#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include"BuildingActionBar.h"
#include<vector>
struct levelInformation {
	int _hp;
	int _upgradeCost;
	std::string spriteFile; 
};

USING_NS_CC;
class Building : public Node
{
public:
    void occupyTiles(float tileX, float tileY);
    std::vector<cocos2d::Vec2> getAttackTiles();

private:
    int _tileX;
    int _tileY;

public:
	static Building* create(const std::string& buildingFile,
        bool isHownTown = true,
        const std::string turfFile = "grass.png",
        float buildingScale = 0.8f);


    virtual ~Building() = default;

    bool isDragging() const { return _isDragging; }
    bool isSelected() const { return _isSelected; }
    cocos2d::Sprite* getTurf() const { return _turf; }
    cocos2d::Sprite* getBuildingSprite() const { return _buildingSprite; }
    void setBuildingSize(int size);
    void setBuildingScale(float scale);
    void setTurfScale(float scale);

    void snapToTile(Node* mapNode, int buildingTileW, int buildingTileH);

    bool isCenterInsideMap(Node* mapNode) const;
    void drawDebugMapRange(Node* mapNode);
    void setBuildingTileSize();
    bool isClickingInTurf(Node* mapNode, cocos2d::EventMouse* e);

    void setTilePosition(Node* mapNode, float tileX, float tileY);
    

    // ========== 标签相关方法 ==========
    void setBuildingName(const std::string& name);    // 设置建筑名称
    std::string getBuildingName() const { return _buildingName; }

    void setShowInfoLabel(bool show);                 // 控制是否显示标签
    void updateInfoLabel();                           // 更新标签内容

    // ========== 等级相关方法 ==========
    void setLevel(int level);                         // 设置等级
    int getLevel() const { return _level; }
	

    // ========== 选择状态控制 ==========
    void setSelected(bool selected);


    // ========== 按钮回调方法（供子类覆盖）==========
    virtual void onInfoButtonClicked();
    virtual void onUpgradeButtonClicked();

  

    // 升级一次
    virtual void upgrade();

    // 设置升级时的图片
    void setUpgradeSprite(int level, const std::string& spriteFile);

    void playUpgradeEffect();
    int getCost() {
        return _cost;
    }
	void setCost(int cost) {
		_cost = cost;
	}
    void setHP(int hp)
    {
        _HP = hp;
    }

    void setMaxLevel(int l){
        _maxLevel = l;
    }
    //血条
    void initHPBar(bool immediateShow);
    void updateHPBar();
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

    
    // ========== 标签相关属性 ==========
    cocos2d::Label* _infoLabel;      // 信息标签
    std::string _buildingName;       // 建筑名称

    // ========== 标签控制方法 ==========
    virtual void createInfoLabel();  // 创建标签
    virtual void showInfoLabel();    // 显示标签
    virtual void hideInfoLabel();    // 隐藏标签
    //==========建筑基础属性===========
    int _level;
    int _HP;
    int _cost;
 
    std::unordered_map<int, levelInformation> _upgradeSprites;  // 等级->图片映射
    void changeBuildingSprite(const std::string& newSpriteFile);  // 更换建筑精灵
  //血条
    cocos2d::ui::LoadingBar* _hpBar = nullptr;
    cocos2d::Sprite* _hpBarBg = nullptr;
    int _currentHP; // 当前血量
    bool _showHPBar = false; // 是否显示血条的控制变量
    // 血条尺寸常量
    const float HP_BAR_WIDTH = 60.0f;
    const float HP_BAR_HEIGHT = 8.0f;
public:
    void reduceHP(float damage);
    bool isAlive() const { return _currentHP > 0; }
public:
    BuildingActionBar* _actionBar = nullptr;
	void setIsHownTown(bool isHownTown) { _isHownTown = isHownTown; }
	bool getIsHownTown() { return _isHownTown; }
    void Building::onDestroyed();
protected:
    int _maxLevel;
    bool _isHownTown= true;

};

#endif // __BUILDING_H__