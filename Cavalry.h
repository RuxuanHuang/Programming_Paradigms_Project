#ifndef __CAVALRY_H__
#define __CAVALRY_H__
#include "Soldier.h"

class Cavalry : public Soldier {
public:
    static Cavalry* create(Vec2 pos, float scale);
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
    Cavalry() : Soldier(Type::CAVALRY) {} // 构造函数指定骑兵类型
};
#endif // __CAVALRY_H__