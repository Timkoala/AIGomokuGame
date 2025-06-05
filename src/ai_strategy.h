#ifndef AI_STRATEGY_H
#define AI_STRATEGY_H

#include <QPoint>
#include <vector>
#include <QString>
#include "game_types.h"

// 前向声明
class Board;
struct Move;

class AIStrategy {
public:
    virtual ~AIStrategy() = default;
    
    // 设置AI难度级别（1-5）
    virtual void setDifficulty(int level) = 0;
    
    // 获取当前难度级别
    virtual int getDifficulty() const { return difficulty; }
    
    // 计算下一步移动
    virtual Move getNextMove(const Board& board, PieceType currentPlayer) = 0;
    
    // 检查该策略是否支持难度调整
    virtual bool supportsDifficulty() const { return true; }

    // 获取策略名称
    virtual QString getName() const = 0;
    
protected:
    int difficulty = 1;  // 默认难度级别
};

#endif // AI_STRATEGY_H 