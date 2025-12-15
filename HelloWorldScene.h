#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include <vector>
#include <string>

class HelloWorld : public cocos2d::Scene
{
private:
    enum {
        GOLD_LABEL_TAG = 200,
        CRYSTAL_LABEL_TAG = 201,
        SELECTED_IMAGE_TAG_BASE = 1000
    };

public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    void openBeachScene();
    CREATE_FUNC(HelloWorld);

private:
    void displayPlayerResources();  // 显示玩家资源
    cocos2d::Sprite* createSelectedImageSprite();
};

class BeachScene : public cocos2d::Layer
{
private:
    cocos2d::Vector<cocos2d::Sprite*> allImages;      // 所有图片
    cocos2d::Vector<cocos2d::Sprite*> visibleImages;  // 当前可见的图片
    cocos2d::Label* currentPageLabel;  // 当前页码标签
    int currentPage;                   // 当前页码
    int touchedImageIndex;             // 当前触摸的图片索引

    enum {
        LEFT_ARROW_TAG = 100,
        RIGHT_ARROW_TAG = 101,
        BORDER_TAG = 102,
        LABEL_TAG = 103,
        GOLD_LABEL_TAG = 200,
        CRYSTAL_LABEL_TAG = 201
    };

public:
    virtual bool init();
    CREATE_FUNC(BeachScene);

private:
    void displayPlayerResources();    // 显示玩家资源
    void updateResourceDisplay();     // 更新资源显示
    void loadAllImages();
    void createImageSelectionArea();   // 创建图片选择区域
    void createImageGallery();         // 创建图片展示
    void updateGalleryDisplay();       // 更新图片显示

    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
};

#endif // __HELLOWORLD_SCENE_H__