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
    None,   // 空位置
    Black,  // 黑棋
    White   // 白棋
};

// AI策略类型
enum class AIStrategyType {
    RuleBased,  // 基于规则的AI
    // 后续可以添加更多AI策略类型
};

#endif // GAME_TYPES_H 