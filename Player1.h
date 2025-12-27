#ifndef __PLAYER1_H__
#define __PLAYER1_H__

#include "Soldier.h"

class Player1 : public Soldier {
public:
    virtual ~Player1();
    static Player1* create(Vec2 pos, float scale);

    virtual bool init(Vec2 pos, float scale) override;
    virtual void moveToTarget(Vec2 targetPos, float duration) override;
    virtual void initWalkAnimation() override;
    virtual void playWalkAnimation() override;
    virtual void stopWalkAnimation() override;

protected:

    // ========== 新增：实现属性纯虚函数 ==========
    virtual void initBaseAttributes() override;
    virtual void updateAttributesByLevel() override;

private:
    Player1() : Soldier(Type::INFANTRY) {} // 初始化类型为步兵
};

#endif // __PLAYER1_H__