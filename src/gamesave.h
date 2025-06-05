#ifndef GAMESAVE_H
#define GAMESAVE_H

#include <QString>
#include <QDateTime>
#include <vector>
#include "game_types.h"

/**
 * @brief 游戏存档类
 */
class GameSave {
public:
    /**
     * @brief 移动记录结构体
     */
    struct Move {
        int row;
        int col;
        Player player;  // 使用 Player 枚举
        Move(int r, int c, Player p) : row(r), col(c), player(p) {}
    };

    /**
     * @brief 游戏存档数据结构
     */
    struct SaveData {
        QDateTime timestamp;                    ///< 存档时间
        bool isAIEnabled;                      ///< 是否为人机对战
        int aiDifficulty;                     ///< AI难度
        int undoLimit;                        ///< 悔棋次数限制
        int remainingUndos;                   ///< 剩余悔棋次数
        std::vector<std::vector<int>> board;  ///< 棋盘状态
        int currentPlayer;                    ///< 当前玩家
        std::vector<Move> history;            ///< 移动历史
    };

    /**
     * @brief 保存游戏状态
     * @param filename 存档文件名
     * @param data 游戏数据
     * @return 是否保存成功
     */
    static bool saveGame(const QString& filename, const SaveData& data);

    /**
     * @brief 加载游戏状态
     * @param filename 存档文件名
     * @param data 加载到的数据结构
     * @return 是否加载成功
     */
    static bool loadGame(const QString& filename, SaveData& data);
};

#endif // GAMESAVE_H 