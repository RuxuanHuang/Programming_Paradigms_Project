
#ifndef __ARCHER_H__
#define __ARCHER_H__
#include "Soldier.h"

class Archer : public Soldier {
public:
    static Archer* create(Vec2 pos, float scale);
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
    Archer() : Soldier(Type::ARCHER) {}
};
#endif // __ARCHER_H__