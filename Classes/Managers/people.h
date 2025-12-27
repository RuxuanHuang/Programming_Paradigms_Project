#ifndef __PEOPLE_H__
#define __PEOPLE_H__

#include "cocos2d.h"
#include <vector>
#include <string>
#include <map>

#include "audio/include/AudioEngine.h"
USING_NS_CC;

// 人物类型枚举
enum class CharacterType
{
    INFANTRY = 0,    // 野蛮人
    ARCHER = 1,     // 弓箭手
    MAGE = 2,       // 巨人
    CAVALRY = 3      // 炸弹人
};

// 人物类
class Character
{
public:
    Character(CharacterType type, const std::string& name, const std::string& image);
    ~Character() {}

    CharacterType getType() const { return m_type; }
    std::string getName() const { return m_name; }
    std::string getImage() const { return m_image; }

    int getCount() const { return m_count; }
    void setCount(int count) { m_count = count; }
    void addCount(int amount = 1) { m_count += amount; }
    void resetCount() { m_count = 0; }

private:
    CharacterType m_type;
    std::string m_name;
    std::string m_image;
    int m_count;
};

// 人口分配管理器
class PopulationManager
{
public:
    static PopulationManager* getInstance();
    static void destroyInstance();

    // 初始化
    void initialize(int totalPopulation);

    // 获取/设置总人口
    int getTotalPopulation() const { return m_totalPopulation; }
    void setTotalPopulation(int population) { m_totalPopulation = population; }
    void addToTotalPopulation(int amount) { m_totalPopulation += amount; }

    // 获取已分配人口
    int getAssignedPopulation() const;

    // 获取剩余可分配人口
    int getAvailablePopulation() const;

    // 人物操作
    bool assignCharacter(CharacterType type, int count = 1);
    bool unassignCharacter(CharacterType type, int count = 1);
    int getCharacterCount(CharacterType type) const;
    void resetAllCharacters();

    // 验证
    bool isValidAssignment(CharacterType type, int count = 1) const;
    bool isValidUnassignment(CharacterType type, int count = 1) const;  // 新增
    bool isReadyForBattle() const { return getAssignedPopulation() > 0; }

    // 获取人物列表
    std::vector<Character*> getAllCharacters() const;
    Character* getCharacter(CharacterType type) const;

    // 保存/加载
    void saveAssignment();
    void loadAssignment();

    // 获取每种人物的消耗
    int getCharacterCost(CharacterType type) const;

private:
    PopulationManager();
    ~PopulationManager();

    static PopulationManager* s_instance;

    int m_totalPopulation;
    Character* m_infantry;
    Character* m_archer;
    Character* m_mage;
    Character* m_cavalry;
    std::map<CharacterType, int> m_characterCosts;

    // 禁止拷贝
    PopulationManager(const PopulationManager&) = delete;
    PopulationManager& operator=(const PopulationManager&) = delete;
};

// 人口分配场景
class PopulationScene : public Scene
{
public:
    static Scene* createScene(int totalPopulation);

    virtual bool init() override;
    CREATE_FUNC(PopulationScene);
    virtual void onEnter() override;
    void setTotalPopulation(int population) { m_totalPopulation = population; }

private:
    void setupUI();
    void updateUI();
    void onCharacterClicked(CharacterType type, int count = 1);  // 修改：添加参数
    void onCharacterRightClicked(CharacterType type, int count = 1);  // 新增
    void onBackClicked(Ref* sender);
    void showFeedback(CharacterType type, const std::string& message, Color3B color);  // 新增

    int m_totalPopulation;
    PopulationManager* m_populationMgr;

    // UI元素
    Label* m_totalLabel;
    Label* m_availableLabel;
    Label* m_infantryLabel;
    Label* m_archerLabel;
    Label* m_mageLabel;
    Label* m_cavalryLabel;
    Sprite* m_infantrySprite;
    Sprite* m_archerSprite;
    Sprite* m_mageSprite;
    Sprite* m_cavalrySprite;
};

#endif // __PEOPLE_H__