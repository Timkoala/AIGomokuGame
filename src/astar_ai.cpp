#include "astar_ai.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <chrono>
#include <random>

AStarAI::AStarAI(int difficulty) : difficulty_(difficulty) {
    // 根据难度设置搜索深度
    maxDepth_ = std::min(1 + difficulty, 4);  // 难度1-5对应深度2-5
}

void AStarAI::setDifficulty(int difficulty) {
    difficulty_ = difficulty;
    maxDepth_ = std::min(1 + difficulty, 4);
}

Move AStarAI::getNextMove(const Board& board, PieceType currentPlayer) {
    auto startTime = std::chrono::steady_clock::now();
    const int MAX_THINK_TIME = 1000 + difficulty_ * 500;  // 基础1秒 + 每难度等级0.5秒

    std::vector<Move> validMoves = getValidMovesInRange(board);
    if (validMoves.empty()) {
        return Move{-1, -1};
    }

    // 如果是第一步，选择靠近中心的位置
    if (validMoves.size() == board.getSize() * board.getSize()) {
        int center = board.getSize() / 2;
        return Move{center, center};
    }

    // 对每个可能的移动进行初步评估
    std::vector<std::pair<Move, int>> scoredMoves;
    scoredMoves.reserve(validMoves.size());
    
    PieceType opponent = (currentPlayer == PieceType::BLACK ? PieceType::WHITE : PieceType::BLACK);
    
    for (const auto& move : validMoves) {
        auto tempBoardState = board.getBoardState();
        
        // 评估进攻价值
        tempBoardState[move.row][move.col] = currentPlayer;
        int attackScore = quickEvaluate(board, tempBoardState, move, currentPlayer);
        
        // 评估防守价值
        tempBoardState[move.row][move.col] = opponent;
        int defenseScore = quickEvaluate(board, tempBoardState, move, opponent);
        
        // 综合评分：进攻价值 + 防守价值的加权
        int finalScore = attackScore;
        
        // 当对手有高威胁时，提高防守权重
        if (defenseScore >= 3000) {  // 对手有活三或以上威胁
            finalScore = std::max(finalScore, defenseScore);  // 取较大值
        } else if (defenseScore >= 800) {  // 对手有活二或以上威胁
            finalScore = std::max(finalScore, attackScore + (defenseScore * 2 / 3));
        } else {
            finalScore = attackScore + (defenseScore / 3);  // 普通情况
        }

        scoredMoves.push_back({move, finalScore});

        // 如果发现必胜着法或必防着法，立即返回
        if (attackScore >= 90000 || defenseScore >= 90000) {
            return move;
        }
    }

    // 随机打乱相同分数的移动
    std::random_device rd;
    std::mt19937 gen(rd());
    std::stable_sort(scoredMoves.begin(), scoredMoves.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // 根据难度保留不同数量的候选移动
    int keepMoves = std::min(6 + difficulty_, static_cast<int>(scoredMoves.size()));
    scoredMoves.resize(keepMoves);

    Move bestMove = scoredMoves[0].first;
    int bestScore = std::numeric_limits<int>::min();
    int alpha = std::numeric_limits<int>::min();
    int beta = std::numeric_limits<int>::max();

    // 对筛选后的移动进行深入搜索
    for (const auto& [move, _] : scoredMoves) {
        auto tempBoardState = board.getBoardState();
        tempBoardState[move.row][move.col] = currentPlayer;
        
        int score = alphaBetaSearch(board, tempBoardState, maxDepth_ - 1, alpha, beta, 
                                  opponent, false);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
        alpha = std::max(alpha, bestScore);

        // 检查时间限制
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        if (elapsedTime > MAX_THINK_TIME) {
            break;
        }
    }

    return bestMove;
}

int AStarAI::quickEvaluate(const Board& board, const std::vector<std::vector<PieceType>>& boardState,
                          const Move& lastMove, PieceType currentPlayer) {
    int score = 0;
    const int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    // 评估最后一步棋的影响
    for (const auto& dir : directions) {
        int lineScore = checkLine(boardState, lastMove.row, lastMove.col, dir[0], dir[1], currentPlayer);
        if (lineScore >= 90000) {
            return 100000;  // 必胜局面
        }
        score += lineScore;
    }

    // 评估周围潜在威胁
    int threatScore = 0;
    const int threatRange = 2;
    for (int dr = -threatRange; dr <= threatRange; ++dr) {
        for (int dc = -threatRange; dc <= threatRange; ++dc) {
            if (dr == 0 && dc == 0) continue;
            
            int newRow = lastMove.row + dr;
            int newCol = lastMove.col + dc;
            
            if (newRow >= 0 && newRow < board.getSize() && 
                newCol >= 0 && newCol < board.getSize()) {
                if (boardState[newRow][newCol] == currentPlayer) {
                    // 检查这个方向上的潜在连线
                    for (const auto& dir : directions) {
                        threatScore += checkLine(boardState, newRow, newCol, dir[0], dir[1], currentPlayer) / 4;
                    }
                }
            }
        }
    }
    
    score += threatScore;
    return score;
}

std::vector<Move> AStarAI::getValidMovesInRange(const Board& board) {
    std::vector<Move> moves;
    int size = board.getSize();
    int searchRange = std::min(1 + difficulty_, 3);  // 限制最大搜索范围为3

    // 遍历所有已放置的棋子
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (board.getPiece(i, j) != PieceType::NONE) {
                // 在该棋子周围搜索空位
                for (int di = -searchRange; di <= searchRange; ++di) {
                    for (int dj = -searchRange; dj <= searchRange; ++dj) {
                        // 使用曼哈顿距离限制搜索范围
                        if (std::abs(di) + std::abs(dj) > searchRange + 1) continue;
                        
                        int newRow = i + di;
                        int newCol = j + dj;
                        
                        if (newRow >= 0 && newRow < size && newCol >= 0 && newCol < size &&
                            board.getPiece(newRow, newCol) == PieceType::NONE) {
                            Move newMove{newRow, newCol};
                            if (std::find_if(moves.begin(), moves.end(),
                                [&](const Move& m) { 
                                    return m.row == newMove.row && m.col == newMove.col; 
                                }) == moves.end()) {
                                moves.push_back(newMove);
                            }
                        }
                    }
                }
            }
        }
    }

    if (moves.empty()) {
        moves.push_back(Move{size / 2, size / 2});
    }

    return moves;
}

int AStarAI::evaluateBoard(const Board& board, const std::vector<std::vector<PieceType>>& boardState,
                          PieceType currentPlayer) {
    int score = 0;
    int size = board.getSize();
    PieceType opponent = (currentPlayer == PieceType::BLACK ? PieceType::WHITE : PieceType::BLACK);
    const int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    
    // 评估所有位置
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (boardState[i][j] != PieceType::NONE) {
                PieceType piece = boardState[i][j];
                int multiplier = (piece == currentPlayer) ? 1 : -1;
                
                // 连子价值
                int lineScoreSum = 0;
                for (const auto& dir : directions) {
                    int lineScore = checkLine(boardState, i, j, dir[0], dir[1], piece);
                    // 如果发现必胜局面，立即返回
                    if (lineScore >= 90000) {
                        return multiplier * 100000;
                    }
                    lineScoreSum += lineScore;
                }
                score += multiplier * lineScoreSum;

                // 位置价值（根据局势动态调整权重）
                if (std::abs(lineScoreSum) < 2000) {
                    int positionScore = calculatePositionScore(board, i, j, piece);
                    score += multiplier * positionScore;
                }
            }
        }
    }

    return score;
}

int AStarAI::checkLine(const std::vector<std::vector<PieceType>>& boardState, int startRow, int startCol, 
                       int dRow, int dCol, PieceType player) {
    int count = 1;
    int empty = 0;
    int size = boardState.size();
    bool blocked = false;
    bool hasGap = false;
    
    // 向一个方向检查
    for (int i = 1; i < 5; ++i) {
        int newRow = startRow + dRow * i;
        int newCol = startCol + dCol * i;
        
        if (newRow < 0 || newRow >= size || newCol < 0 || newCol >= size) {
            blocked = true;
            break;
        }
        
        PieceType piece = boardState[newRow][newCol];
        if (piece == player) {
            if (empty > 0) hasGap = true;
            count++;
        } else if (piece == PieceType::NONE) {
            if (empty == 0 && i <= 4) {
                empty++;
                continue;
            }
            break;
        } else {
            blocked = true;
            break;
        }
    }
    
    int backEmpty = 0;
    bool backBlocked = false;
    
    // 向相反方向检查
    for (int i = 1; i < 5; ++i) {
        int newRow = startRow - dRow * i;
        int newCol = startCol - dCol * i;
        
        if (newRow < 0 || newRow >= size || newCol < 0 || newCol >= size) {
            backBlocked = true;
            break;
        }
        
        PieceType piece = boardState[newRow][newCol];
        if (piece == player) {
            if (backEmpty > 0) hasGap = true;
            count++;
        } else if (piece == PieceType::NONE) {
            if (backEmpty == 0 && i <= 4) {
                backEmpty++;
                continue;
            }
            break;
        } else {
            backBlocked = true;
            break;
        }
    }
    
    empty += backEmpty;
    blocked = blocked && backBlocked;
    
    // 根据连子数和空位数计算分数
    if (count >= 5) return 100000;  // 胜利
    
    // 基础分数
    int baseScore;
    if (blocked) {
        if (count == 4) return 3000;  // 死四
        if (count == 3) return 300;   // 死三
        if (count == 2) return 30;    // 死二
        baseScore = count * 8;
    } else {
        if (count == 4) {
            if (empty >= 2) return 20000;  // 活四
            baseScore = 8000;              // 单活四
        } else if (count == 3) {
            if (empty >= 2) return 3000;   // 活三
            baseScore = 800;               // 单活三
        } else if (count == 2) {
            if (empty >= 2) return 200;    // 活二
            baseScore = 50;                // 单活二
        } else {
            baseScore = count * 15;
        }
    }
    
    // 有间断的情况分数降低
    if (hasGap) {
        baseScore = baseScore * 2 / 3;
    }
    
    return baseScore;
}

int AStarAI::alphaBetaSearch(const Board& board, std::vector<std::vector<PieceType>>& boardState,
                            int depth, int alpha, int beta, PieceType currentPlayer, bool isMaximizing) {
    // 创建临时Board对象来检查胜负
    Board tempBoard;
    tempBoard.setBoardState(boardState);

    // 到达叶子节点或游戏结束
    if (depth == 0) {
        return evaluateBoard(board, boardState, currentPlayer);
    }

    std::vector<Move> validMoves = getValidMovesInRange(board);
    if (validMoves.empty()) {
        return evaluateBoard(board, boardState, currentPlayer);
    }

    if (isMaximizing) {
        int maxScore = std::numeric_limits<int>::min();
        for (const auto& move : validMoves) {
            // 保存原始状态
            auto originalPiece = boardState[move.row][move.col];
            
            // 尝试移动
            boardState[move.row][move.col] = currentPlayer;
            
            int score = alphaBetaSearch(board, boardState, depth - 1, alpha, beta,
                                      (currentPlayer == PieceType::BLACK ? PieceType::WHITE : PieceType::BLACK),
                                      false);
            
            // 恢复原始状态
            boardState[move.row][move.col] = originalPiece;
            
            maxScore = std::max(maxScore, score);
            alpha = std::max(alpha, score);
            if (beta <= alpha) {
                break;  // Beta剪枝
            }
        }
        return maxScore;
    } else {
        int minScore = std::numeric_limits<int>::max();
        for (const auto& move : validMoves) {
            // 保存原始状态
            auto originalPiece = boardState[move.row][move.col];
            
            // 尝试移动
            boardState[move.row][move.col] = currentPlayer;
            
            int score = alphaBetaSearch(board, boardState, depth - 1, alpha, beta,
                                      (currentPlayer == PieceType::BLACK ? PieceType::WHITE : PieceType::BLACK),
                                      true);
            
            // 恢复原始状态
            boardState[move.row][move.col] = originalPiece;
            
            minScore = std::min(minScore, score);
            beta = std::min(beta, score);
            if (beta <= alpha) {
                break;  // Alpha剪枝
            }
        }
        return minScore;
    }
}

int AStarAI::calculatePositionScore(const Board& board, int row, int col, PieceType player) {
    int size = board.getSize();
    int centerValue = size / 2;
    
    // 使用曼哈顿距离计算到中心的距离
    int distanceToCenter = std::abs(row - centerValue) + std::abs(col - centerValue);
    
    // 基础分数：越靠近中心分数越高
    int baseScore = 120 - (distanceToCenter * 8);
    
    // 根据周围棋子情况调整分数
    int neighborScore = 0;
    const int searchRange = 2;
    
    for (int dr = -searchRange; dr <= searchRange; ++dr) {
        for (int dc = -searchRange; dc <= searchRange; ++dc) {
            if (dr == 0 && dc == 0) continue;
            
            int newRow = row + dr;
            int newCol = col + dc;
            
            if (newRow >= 0 && newRow < size && newCol >= 0 && newCol < size) {
                PieceType piece = board.getPiece(newRow, newCol);
                if (piece != PieceType::NONE) {
                    // 相邻位置的己方子分数高一些
                    if (std::abs(dr) + std::abs(dc) == 1) {
                        neighborScore += (piece == player) ? 15 : 10;
                    }
                    // 次相邻位置分数较低
                    else if (std::abs(dr) + std::abs(dc) == 2) {
                        neighborScore += (piece == player) ? 8 : 5;
                    }
                }
            }
        }
    }
    
    // 最终分数为基础分数和邻近分数的加权和
    return std::max(0, baseScore + (neighborScore / 2));
} 