#include "rule_based_ai.h"
#include <algorithm>
#include <random>
#include <cstdlib>  // 为abs函数添加头文件

RuleBasedAI::RuleBasedAI() {
    difficulty = 1;  // 默认难度为1
}

void RuleBasedAI::setDifficulty(int level) {
    difficulty = std::clamp(level, 1, 5);
}

Move RuleBasedAI::getNextMove(const Board& board, PieceType currentPlayer) {
    auto emptyPositions = getEmptyPositions(board);
    if (emptyPositions.empty()) {
        return Move{-1, -1};
    }

    // 如果是第一步，选择靠近中心的位置
    if (emptyPositions.size() == board.getSize() * board.getSize()) {
        int center = board.getSize() / 2;
        return Move{center, center};
    }

    // 根据难度级别评估不同的策略
    std::vector<std::pair<int, Move>> scoredMoves;
    const int boardSize = board.getSize();
    const int boardCenter = boardSize / 2;
    
    for (const auto& pos : emptyPositions) {
        int score = evaluatePosition(board, pos.row, pos.col, currentPlayer);
        
        // 根据难度增加评估的复杂度
        if (difficulty >= 2) {
            // 考虑对手的威胁
            score = std::max(score, 
                evaluatePosition(board, pos.row, pos.col, 
                    currentPlayer == PieceType::BLACK ? PieceType::WHITE : PieceType::BLACK));
        }
        
        if (difficulty >= 3) {
            // 考虑位置的战略价值
            int centerDistance = abs(pos.row - boardCenter) + 
                               abs(pos.col - boardCenter);
            score += (boardSize - centerDistance) * 2;
        }
        
        if (difficulty >= 4) {
            // 考虑多方向的威胁
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    score += checkLine(board, pos.row, pos.col, dx, dy, currentPlayer) * 10;
                }
            }
        }
        
        scoredMoves.emplace_back(score, pos);
    }

    // 根据分数排序
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // 难度5时始终选择最佳移动，其他难度有一定随机性
    if (difficulty == 5) {
        return scoredMoves[0].second;
    } else {
        // 根据难度选择前N个最佳移动中的一个
        int range = std::max(1, 6 - difficulty);
        range = std::min(range, static_cast<int>(scoredMoves.size()));
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, range - 1);
        return scoredMoves[dis(gen)].second;
    }
}

int RuleBasedAI::evaluatePosition(const Board& board, int row, int col, PieceType currentPlayer) {
    int score = 0;
    
    // 检查八个方向
    const int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
        {0, 1}, {1, -1}, {1, 0}, {1, 1}
    };

    for (const auto& dir : directions) {
        int count = checkLine(board, row, col, dir[0], dir[1], currentPlayer);
        
        // 根据连子数量评分
        switch (count) {
            case 5: score += 100000; break;  // 胜利
            case 4: score += 10000; break;   // 四子连珠
            case 3: score += 1000; break;    // 三子连珠
            case 2: score += 100; break;     // 两子连珠
            case 1: score += 10; break;      // 单子
        }
    }
    
    return score;
}

int RuleBasedAI::checkLine(const Board& board, int row, int col, int dRow, int dCol,
                          PieceType currentPlayer) {
    int count = 1;  // 包含当前位置
    int r, c;
    
    // 向一个方向检查
    r = row + dRow;
    c = col + dCol;
    while (isValidPosition(r, c) && board.getPiece(r, c) == currentPlayer) {
        count++;
        r += dRow;
        c += dCol;
    }
    
    // 向相反方向检查
    r = row - dRow;
    c = col - dCol;
    while (isValidPosition(r, c) && board.getPiece(r, c) == currentPlayer) {
        count++;
        r -= dRow;
        c -= dCol;
    }
    
    return count;
}

std::vector<Move> RuleBasedAI::getEmptyPositions(const Board& board) {
    std::vector<Move> emptyPositions;
    int size = board.getSize();
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board.getPiece(i, j) == PieceType::NONE) {
                emptyPositions.emplace_back(Move{i, j});
            }
        }
    }
    return emptyPositions;
}

bool RuleBasedAI::isValidPosition(int row, int col) const {
    return row >= 0 && row < 15 && col >= 0 && col < 15;  // 假设棋盘大小为15x15
} 