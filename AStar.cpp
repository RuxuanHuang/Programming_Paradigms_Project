#include "AStar.h"
#include "BattleMapLogic.h"
#include <algorithm>
#include <limits>

USING_NS_CC;

struct AStarNode
{
    int x, y;
    int g, h;
    AStarNode* parent;

    int f() const { return g + h; }
};

static int heuristic(int x1, int y1, int x2, int y2)
{
    return abs(x1 - x2) + abs(y1 - y2);
}

static AStarNode* findAStarNode(std::vector<AStarNode*>& list, int x, int y)
{
    for (auto n : list)
        if (n->x == x && n->y == y)
            return n;
    return nullptr;
}

std::vector<Vec2> findPath(const Vec2& startTile, const Vec2& endTile)
{
    std::vector<AStarNode*> openList;
    std::vector<AStarNode*> closedList;
    AStarNode* start = new AStarNode{
        (int)startTile.x,
        (int)startTile.y,
        0,
        heuristic(startTile.x, startTile.y, endTile.x, endTile.y),
        nullptr
    };

    openList.push_back(start);

    const int dirs[4][2] = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
    };

    while (!openList.empty())
    {
        // 取 f 最小
        auto currentIt = std::min_element(openList.begin(), openList.end(),
            [](AStarNode* a, AStarNode* b) {
                return a->f() < b->f();
            });

        AStarNode* current = *currentIt;

        // 到达终点
        if (current->x == endTile.x && current->y == endTile.y)
        {
            std::vector<Vec2> path;
            while (current)
            {
                path.push_back(Vec2(current->x, current->y));
                current = current->parent;
            }
            std::reverse(path.begin(), path.end());

            // 清理内存
            for (auto n : openList) delete n;
            for (auto n : closedList) delete n;

            return path;
        }

        openList.erase(currentIt);
        closedList.push_back(current);

        // 扩展邻居
        for (int i = 0; i < 4; i++)
        {
            int nx = current->x + dirs[i][0];
            int ny = current->y + dirs[i][1];

            if (!isWalkable(nx, ny)) {
                continue;
            }

            if (findAStarNode(closedList, nx, ny)) {
                continue;
            }

            int newG = current->g + 1;
            AStarNode* neighbor = findAStarNode(openList, nx, ny);

            if (!neighbor)
            {
                neighbor = new AStarNode{
                    nx, ny,
                    newG,
                    heuristic(nx, ny, endTile.x, endTile.y),
                    current
                };
                openList.push_back(neighbor);
            }
            else if (newG < neighbor->g)
            {
                neighbor->g = newG;
                neighbor->parent = current;
            }
        }
    }

    // 没路
    for (auto n : openList) delete n;
    for (auto n : closedList) delete n;

    return {};
}
