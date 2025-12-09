#ifndef __COC_SCENE_H__
#define __COC_SCENE_H__

#include "cocos2d.h"
#include <vector> 
USING_NS_CC;

class Player; // 前置声明
class COC : public Scene
{
public:
    static Scene* createScene();
    virtual bool init();
    void menuCloseCallback(Ref* pSender);
    CREATE_FUNC(COC);

private:
    int _currentSoldierCount = 0;
    const int MAX_SOLDIER_COUNT = 10;
    std::vector<Player*> _soldierList; // 存储士兵列表
};

#endif // __COC_SCENE_H__