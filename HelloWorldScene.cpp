#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "audio/include/AudioEngine.h"
#include "resources.h"
USING_NS_CC;

// 定义标签常量
const int GOLD_LABEL_TAG = 1001;
const int CRYSTAL_LABEL_TAG = 1002;
const int LEFT_ARROW_TAG = 1003;
const int RIGHT_ARROW_TAG = 1004;
const int LABEL_TAG = 1005;

class BeachScene;

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(HelloWorld);

    // 音频控制方法
    static void playBackgroundMusic();
    static void stopBackgroundMusic();

    virtual void onEnter() override;
    virtual void onExit() override;

    void displayPlayerResources();
    void openBeachScene();

private:
    cocos2d::Sprite* createSelectedImageSprite();
    static int s_backgroundAudioID;  // 主场景背景音乐的音频ID
};

class BeachScene : public cocos2d::Layer
{
public:
    virtual bool init() override;

    CREATE_FUNC(BeachScene);

    virtual void onEnter() override;
    virtual void onExit() override;

private:
    void displayPlayerResources();
    void loadAllImages();
    void createImageSelectionArea();
    void createImageGallery();
    void updateResourceDisplay();
    void updateGalleryDisplay();

    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    // 商店音频控制
    static void playShopBackgroundMusic();
    static void stopShopBackgroundMusic();

private:
    cocos2d::Vector<cocos2d::Sprite*> allImages;
    cocos2d::Vector<cocos2d::Sprite*> visibleImages;
    cocos2d::Label* currentPageLabel = nullptr;
    int currentPage = 0;
    int touchedImageIndex = -1;
    ResourceManager* m_resourceManager;
    static int s_shopBackgroundAudioID;  // 商店场景背景音乐的音频ID
};

#endif // __HELLOWORLD_SCENE_H__
