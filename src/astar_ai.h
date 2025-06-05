#ifndef ASTAR_AI_H
#define ASTAR_AI_H

#include "ai_strategy.h"
#include "game_types.h"
#include "board.h"
#include <vector>
#include <utility>

class AStarAI : public AIStrategy {
public:
    AStarAI(int difficulty = 1);
    void setDifficulty(int level) override;
    Move getNextMove(const Board& board, PieceType currentPlayer) override;
    QString getName() const override { return "AStar"; }

private:
    struct SearchNode {
        Move move;
        int score;
        int depth;
        
        SearchNode(const Move& m, int s, int d) 
            : move(m), score(s), depth(d) {}
    };

    int difficulty_;
    int maxDepth_;
    const int MAX_SCORE = 1000000;

    // 核心搜索函数
    int alphaBetaSearch(const Board& board, std::vector<std::vector<PieceType>>& boardState,
                       int depth, int alpha, int beta, PieceType currentPlayer, bool isMaximizing);
    
    // 估价函数
    int evaluateBoard(const Board& board, const std::vector<std::vector<PieceType>>& boardState,
                     PieceType currentPlayer);
    
    // 获取搜索范围内的所有可能移动
    std::vector<Move> getValidMovesInRange(const Board& board);
    
    // 计算位置分数
    int calculatePositionScore(const Board& board, int row, int col, PieceType player);
    
    // 检查连子情况
    int checkLine(const std::vector<std::vector<PieceType>>& boardState, int startRow, int startCol, 
                 int dRow, int dCol, PieceType player);

    /**
     * @brief 快速评估一个移动的价值
     * @param board 棋盘对象
     * @param boardState 当前棋盘状态
     * @param lastMove 最后一步移动
     * @param currentPlayer 当前玩家
     * @return 评分
     */
    int quickEvaluate(const Board& board, const std::vector<std::vector<PieceType>>& boardState,
                      const Move& lastMove, PieceType currentPlayer);
};

#endif // ASTAR_AI_H 