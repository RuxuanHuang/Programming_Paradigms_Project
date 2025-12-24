#ifndef __STORE_H__
#define __STORE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "AudioEngine.h"

USING_NS_CC;

// 商店卡片数据结构
struct StoreCardData {
    std::string cardName;      // 卡片名称
    std::string iconPath;      // 图标路径
    int price;                 // 价格
    std::string costType;     // 货币类型(金币/圣水）
    bool isLocked;            // 是否锁定
};

class Store : public Scene {
public:
    // 添加回调函数类型定义
    typedef std::function<void(const std::string& cardName)> CardSelectCallback;

    CREATE_FUNC(Store);
    virtual bool init() override;

    // 设置卡片选择回调
    void setCardSelectCallback(const CardSelectCallback& callback) {
        _cardSelectCallback = callback;
    }
    

private:
    // 初始化数据
    void initCardData();
    // 创建UI
    void createUI();
    // 创建卡片
    void createCard(const StoreCardData& data, Vec2 position);
    // 处理卡片点击
    void onCardClicked(const std::string& cardName);

    Vector<Sprite*> cardSprites;
    std::vector<StoreCardData> cardDataList;
    CardSelectCallback _cardSelectCallback; // 卡片选择回调
    
};

#endif // __STORE_H__