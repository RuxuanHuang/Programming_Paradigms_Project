#include "cocos2d.h"
#include <vector>
USING_NS_CC;

class Player : public Sprite
{
public:
    static Player* create(const std::string& imgPath, Vec2 pos, float scale);
    bool init(const std::string& imgPath, Vec2 pos, float scale);
    void moveToTarget(Vec2 targetPos, float duration = 2.0f);
    void initWalkAnimation(const std::vector<std::string>& walkFrames);
    void playWalkAnimation();
    void stopWalkAnimation();

private:
    Action* _moveAction = nullptr;
    Animation* _walkAnimation = nullptr;
    Action* _walkAction = nullptr;
    std::vector<std::string> _walkFrames;
    std::string _idleImgPath;
};