#ifndef __SOLDIER_H__
#define __SOLDIER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h" // 引入UI组件头文件
#include <functional>
#include <vector>
#include "Building.h"
USING_NS_CC;

class BattleScene1;
class Soldier : public Sprite {
public:
    void moveToBuilding(Building* target);
    Vec2 chooseTargetTile(Building* b);

    void setTilePosition(const cocos2d::Vec2& tilePos) {
        _tilePos = tilePos;
        CCLOG("士兵设置格子位置: (%.1f, %.1f)", _tilePos.x, _tilePos.y);
    }

private:
    cocos2d::Vec2 _tilePos;

//////////////////////

public:
    enum class Type {
        INFANTRY,   // 步兵（对应原Player1）
        ARCHER,     // 弓箭手
        CAVALRY,    // 骑兵
        MAGE        // 法师
    };

    // 新增：士兵状态枚举
    enum class State {
        MOVING,       // 移动中（未进入攻击范围）
        WAITING,      // 已进入范围，排队等待受击
        UNDER_ATTACK, // 正在受击（每秒掉血）
        DEAD          // 已死亡
    };
    // 显式显示血条
    void showHPBar() {
        if (_hpBar) _hpBar->setVisible(true);
        if (auto hpBg = _hpBar->getParent()->getChildByTag(100)) {
            hpBg->setVisible(true);
        }
    }

    // 显式隐藏血条
    void hideHPBar() {
        if (_hpBar) _hpBar->setVisible(false);
        if (auto hpBg = _hpBar->getParent()->getChildByTag(100)) {
            hpBg->setVisible(false);
        }
    }
    State getState() const { return _state; }
    void setState(State state) { _state = state; }
    bool isAlive() const { return _state != State::DEAD; } // 存活判断

    // 新增：受击相关方法
    void startTakeDamagePerSecond(float damagePerSec); // 开始每秒扣血
    void stopTakeDamage(); // 停止扣血
    void onTakeDamage(float damage); // 单次扣血（复用原有扣血逻辑）

    std::function<void(Soldier*)> onReachTarget; // 到达目标回调

    static Soldier* create(Type type, Vec2 pos, float scale);

    virtual bool init(Vec2 pos, float scale) = 0;
    virtual void moveToTarget(Vec2 targetPos, float duration) = 0;
    virtual void initWalkAnimation() = 0;
    virtual void playWalkAnimation() = 0;
    virtual void stopWalkAnimation() = 0;

    Type getType() const { return _type; }

    // ========== 新增属性相关方法 ==========
   // 获取/设置等级（临时占位，后续替换为队友接口）
    int getLevel() const { return _level; }
    void setLevel(int level) {
        _level = level;
        updateAttributesByLevel(); // 等级变化时自动更新属性
    }

    // 生命值相关
    float getCurrentHP() const { return _currentHP; }
    float getMaxHP() const { return _maxHP; }
   

    // 攻击力相关
    float getAttack() const { return _attack; }

    // 绑定场景（用于后续获取队友接口的等级）
    void bindScene(BattleScene1* scene) { _scene = scene; }

protected:
    // 翻转精灵以改变朝向
    void setFlippedX(bool flipped);

    // 新增：控制血条显隐
    void setHPBarVisible(bool visible) {
        if (_hpBar) _hpBar->setVisible(visible);
    }

    cocos2d::ui::LoadingBar* _hpBar; // 血条（LoadingBar）
    float _hpBarWidth = 40.0f;       // 血条宽度
    float _hpBarHeight = 5.0f;       // 血条高度

    // 初始化血条（LoadingBar版）
    void initHPBar();
    // 更新血条百分比
    void updateHPBar();
    void reduceHP(float damage);

    State _state = State::MOVING; // 初始状态：移动中
    float _damagePerSec = 0.0f;   // 每秒受击伤害值


    Soldier(Type type) : _type(type), _level(1) {} // 初始1级

    virtual bool initBase(const std::string& idleImgPath, cocos2d::Vec2 pos, float scale);

    Type _type;
    std::string _idleImgPath;
    Animation* _walkAnimation = nullptr;
    Action* _walkAction = nullptr;
    Action* _moveAction = nullptr;
    std::vector<std::string> _walkFrames;

    // ========== 新增属性成员 ==========
    BattleScene1* _scene = nullptr;       // 绑定的场景指针
    int _level;                  // 等级（临时，后续从队友接口取）

    // 生命值
    float _maxHP;                // 最大生命值（随等级变化）
    float _currentHP;            // 当前生命值

    // 攻击力
    float _attack;               // 攻击力（随等级变化）

    // ========== 纯虚函数（各兵种实现自己的成长公式） ==========
    // 初始化1级基础属性（每个兵种不同）
    virtual void initBaseAttributes() = 0;
    // 根据等级更新属性（核心成长逻辑）
    virtual void updateAttributesByLevel() = 0;
    // 死亡回调（可扩展死亡动画/销毁逻辑）
    void onDeath();
};

#endif // __SOLDIER_H__