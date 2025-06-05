#ifndef BOARD_H
#define BOARD_H

#include <QWidget>
#include <vector>
#include <random>
#include <stack>
#include "game_types.h"
#include "gamesave.h"

/**
 * @brief 棋盘类
 * 
 * Board类负责实现五子棋的核心游戏逻辑，包括：
 * - 棋盘的绘制
 * - 落子的处理
 * - 胜负的判断
 * - 游戏状态的管理
 */
class Board : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit Board(QWidget *parent = nullptr);

    /**
     * @brief 重置游戏
     * @param enableAI 是否启用AI
     * @param difficulty AI难度（1-5）
     * @param undoLimit 允许的悔棋次数
     */
    void resetGame(bool enableAI = false, int difficulty = 3, int undoLimit = 3);

    /**
     * @brief 保存当前游戏状态
     * @param filename 存档文件名
     * @return 是否保存成功
     */
    bool saveGameState(const QString& filename);

    /**
     * @brief 加载游戏状态
     * @param filename 存档文件名
     * @return 是否加载成功
     */
    bool loadGameState(const QString& filename);

protected:
    /**
     * @brief 绘制事件处理函数
     * @param event 绘制事件对象
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief 鼠标按下事件处理函数
     * @param event 鼠标事件对象
     */
    void mousePressEvent(QMouseEvent *event) override;

private:
    static const int BOARD_SIZE = 15;    ///< 棋盘大小（15x15）
    static const int CELL_SIZE = 35;     ///< 每个格子的大小（像素）
    static const int MARGIN = 20;        ///< 棋盘边距（像素）
    
    /**
     * @brief 移动记录结构体
     */
    struct Move {
        int row;
        int col;
        Player player;
        Move(int r, int c, Player p) : row(r), col(c), player(p) {}
    };

    std::vector<std::vector<Player>> board;  ///< 棋盘状态数组
    Player currentPlayer;                    ///< 当前玩家
    bool gameOver;                          ///< 游戏是否结束
    bool aiEnabled;                         ///< 是否启用AI
    int aiDifficulty;                       ///< AI难度等级
    std::mt19937 rng;                       ///< 随机数生成器
    
    int remainingUndos;                     ///< 剩余悔棋次数
    std::stack<Move> moveHistory;           ///< 移动历史记录

    /**
     * @brief 绘制棋盘
     * @param painter 画笔对象
     */
    void drawBoard(QPainter &painter);

    /**
     * @brief 绘制棋子
     * @param painter 画笔对象
     */
    void drawPieces(QPainter &painter);

    /**
     * @brief 检查是否获胜
     * @param row 行号
     * @param col 列号
     * @return 是否获胜
     */
    bool checkWin(int row, int col);

    /**
     * @brief 棋盘坐标转像素坐标
     * @param row 行号
     * @param col 列号
     * @return 像素坐标
     */
    QPoint boardToPixel(int row, int col) const;

    /**
     * @brief 像素坐标转棋盘坐标
     * @param x X坐标
     * @param y Y坐标
     * @return 棋盘坐标
     */
    QPoint pixelToBoard(int x, int y) const;

    /**
     * @brief 显示游戏结束对话框
     * @param winner 获胜方
     */
    void showGameOver(Player winner);

    /**
     * @brief AI下棋
     */
    void makeAIMove();

    /**
     * @brief 评估位置分数
     * @param row 行号
     * @param col 列号
     * @param player 待评估的玩家
     * @return 位置分数
     */
    int evaluatePosition(int row, int col, Player player);

    /**
     * @brief 获取所有可能的移动位置
     * @return 可能的移动位置列表
     */
    std::vector<std::pair<int, int>> getAvailableMoves();

    /**
     * @brief 执行悔棋操作
     * @return 是否成功悔棋
     */
    bool undoMove();
};

#endif // BOARD_H 