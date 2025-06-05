#ifndef GAME_TYPES_H
#define GAME_TYPES_H

/**
 * @brief 玩家枚举
 */
enum class Player { 
    None,   ///< 空位
    Black,  ///< 黑方
    White   ///< 白方
};

// 棋子类型
enum class PieceType {
    NONE,   // 空位置
    BLACK,  // 黑棋
    WHITE   // 白棋
};

// AI策略类型
enum class AIStrategyType {
    RuleBased,  // 基于规则的AI
    AStar,      // A*启发式搜索AI
    // 后续可以添加更多AI策略类型
};

/**
 * @brief 移动结构体
 */
struct Move {
    int row;        ///< 行号
    int col;        ///< 列号
    PieceType player = PieceType::NONE;  ///< 玩家（可选）

    Move(int r = -1, int c = -1, PieceType p = PieceType::NONE) 
        : row(r), col(c), player(p) {}
};

#endif // GAME_TYPES_H 