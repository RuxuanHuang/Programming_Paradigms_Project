#ifndef __MAGE_H__
#define __MAGE_H__
#include "Soldier.h"

class Mage : public Soldier {
public:
    static Mage* create(Vec2 pos, float scale);
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
    Mage() : Soldier(Type::MAGE) {} // 构造函数指定法师类型
};
#endif // __MAGE_H__