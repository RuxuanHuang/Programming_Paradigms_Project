#pragma once
#ifndef __INFANTRY_H__
#define __INFANTRY_H__

#include "Soldier.h"

class Infantry : public Soldier {
public:
    // 显式构造函数，调用基类的带参构造函数
    Infantry() : Soldier(Type::INFANTRY) {}  // 假设Type::INFANTRY是步兵类型
    static Infantry* create(Vec2 pos, float scale);
    virtual void moveToTarget(Vec2 targetPos, float duration) override;
    virtual void initWalkAnimation() override;
    virtual void playWalkAnimation() override;
    virtual void stopWalkAnimation() override;
protected:
    // 关键：补全基类纯虚函数的实现声明（之前缺失这部分）
    virtual void initBaseAttributes() override;
    virtual void updateAttributesByLevel() override;
private:
    bool init(Vec2 pos, float scale);
};

#endif // __INFANTRY_H__