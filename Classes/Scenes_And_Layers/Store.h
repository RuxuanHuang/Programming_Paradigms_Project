#ifndef __STORE_H__
#define __STORE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "audio/include/AudioEngine.h"
USING_NS_CC;


static int g_selectedImageIndex = -1;
static cocos2d::Rect g_selectedImageRect = cocos2d::Rect::ZERO;
static cocos2d::Color3B g_selectedImageColor = cocos2d::Color3B::WHITE;
static cocos2d::Texture2D* g_selectedTexture = nullptr;
static cocos2d::Rect g_selectedTextureRect = cocos2d::Rect::ZERO;
static std::string g_selectedImageName = "";

const int GOLD_LABEL_TAG = 1001;
const int CRYSTAL_LABEL_TAG = 1002;
const int LEFT_ARROW_TAG = 1003;
const int RIGHT_ARROW_TAG = 1004;
const int LABEL_TAG = 1005;

// 玩家资源
static int g_playerGold = 100;
static int g_playerCrystal = 50;

// 图片价格表
static const std::vector<int> g_imageGoldPrices = { 20, 30, 25, 40, 35, 45, 50, 60 };
static const std::vector<int> g_imageCrystalPrices = { 5, 8, 6, 10, 7, 12, 15, 20 };

class Store : public Layer
{
public:
    virtual bool init() override;

    CREATE_FUNC(Store);

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

    

private:
    cocos2d::Vector<cocos2d::Sprite*> allImages;
    cocos2d::Vector<cocos2d::Sprite*> visibleImages;
    cocos2d::Label* currentPageLabel = nullptr;
    int currentPage = 0;
    int touchedImageIndex = -1;

    
}; 
#endif 
