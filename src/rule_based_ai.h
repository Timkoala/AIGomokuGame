#ifndef RULE_BASED_AI_H
#define RULE_BASED_AI_H

#include "ai_strategy.h"
#include "board.h"

class RuleBasedAI : public AIStrategy {
public:
    RuleBasedAI();
    
    void setDifficulty(int level) override;
    Move getNextMove(const Board& board, PieceType currentPlayer) override;
    QString getName() const override { return "RuleBased"; }

private:
    // 评估某个位置的分数
    int evaluatePosition(const Board& board, int row, int col, PieceType currentPlayer);
    
    // 检查连子数量（横、竖、斜）
    int checkLine(const Board& board, int row, int col, int dRow, int dCol,
                 PieceType currentPlayer);
    
    // 获取空位置列表
    std::vector<Move> getEmptyPositions(const Board& board);
    
    // 检查是否在边界内
    bool isValidPosition(int row, int col) const;
};

#endif // RULE_BASED_AI_H 