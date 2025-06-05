#ifndef RULE_BASED_AI_H
#define RULE_BASED_AI_H

#include "ai_strategy.h"

class RuleBasedAI : public AIStrategy {
public:
    RuleBasedAI();
    
    void setDifficulty(int level) override;
    int getDifficulty() const override;
    QPoint getNextMove(const std::vector<std::vector<PieceType>>& board,
                      PieceType currentPlayer) override;
    bool supportsDifficulty() const override { return true; }

private:
    // 评估某个位置的分数
    int evaluatePosition(const std::vector<std::vector<PieceType>>& board,
                        int row, int col, PieceType currentPlayer);
    
    // 检查连子数量（横、竖、斜）
    int checkLine(const std::vector<std::vector<PieceType>>& board,
                 int row, int col, int dRow, int dCol,
                 PieceType currentPlayer);
    
    // 获取空位置列表
    std::vector<QPoint> getEmptyPositions(const std::vector<std::vector<PieceType>>& board);
    
    // 检查是否在边界内
    bool isValidPosition(int row, int col) const;
};

#endif // RULE_BASED_AI_H 