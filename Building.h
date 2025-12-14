#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
USING_NS_CC;
class Building : public Node
{
public:
    static Building* create(const std::string& buildingFile,
        const std::string& turfFile = "grass.png",
        float buildingScale = 0.7f);

    static Building* create(const std::string& buildingFile) {
        return create(buildingFile, "grass.png", 0.7f);
    }

    virtual ~Building() = default;

    bool isDragging() const { return _isDragging; }
    cocos2d::Sprite* getTurf() const { return _turf; }
    cocos2d::Sprite* getBuildingSprite() const { return _buildingSprite; }

    void setBuildingScale(float scale);
    void setTurfScale(float scale);

   

    void showDiamondDebug(bool show);
    void setDiamondDebugParams(float widthRatio, float heightRatio);

    void snapToTile(Node* mapNode, int buildingTileW, int buildingTileH);

    bool isCenterInsideMap(Node* mapNode) const;
    void drawDebugMapRange(Node* mapNode);
    void setBuildingTileSize(int tileWidthCount, int tileHeightCount);
    bool isClickingInTurf(Node* mapNode, cocos2d::EventMouse* e);

protected:
    Vec2 _dragStartPos;


    Building();

    virtual bool init(const std::string& buildingFile,
        const std::string& turfFile,
        float buildingScale);

    cocos2d::Sprite* _turf;
    cocos2d::Sprite* _buildingSprite;
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

    bool isClickOnGreenDiamond(cocos2d::EventMouse* e);
    bool pointInPolygon(const cocos2d::Vec2& point, cocos2d::Vec2 polygon[], int n);
    void getDiamondVertices(cocos2d::Vec2 vertices[4]);
    void setupBuildingOnTurf();
    void updateDebugDisplay();


    void updateBuildingAppearance();
    int _level;
};

#endif // __BUILDING_H__