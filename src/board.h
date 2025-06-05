#ifndef BOARD_H
#define BOARD_H

#include <QWidget>
#include <vector>
#include <random>
#include <stack>
#include <memory>
#include "game_types.h"
#include "gamesave.h"
#include "ai_strategy.h"

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
     * @param aiStrategy AI策略名称
     * @param difficulty AI难度（1-5）
     * @param undoLimit 允许的悔棋次数
     * @param playerPieceType 玩家选择的棋子颜色（仅在AI模式下有效）
     */
    void resetGame(bool enableAI = false, const QString& aiStrategy = "RuleBased",
                  int difficulty = 3, int undoLimit = 3, 
                  PieceType playerPieceType = PieceType::BLACK);

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

    /**
     * @brief 设置AI策略
     * @param strategyName AI策略名称
     */
    void setAIStrategy(const QString& strategyName);

    /**
     * @brief 获取棋盘大小
     */
    int getSize() const { return BOARD_SIZE; }

    /**
     * @brief 获取指定位置的棋子类型
     */
    PieceType getPiece(int row, int col) const { return board[row][col]; }

    /**
     * @brief 在指定位置放置棋子
     */
    void placePiece(int row, int col, PieceType piece) { board[row][col] = piece; }

    /**
     * @brief 获取棋盘状态
     */
    std::vector<std::vector<PieceType>> getBoardState() const { return board; }

    /**
     * @brief 设置棋盘状态
     */
    void setBoardState(const std::vector<std::vector<PieceType>>& newBoard) { board = newBoard; }

    /**
     * @brief 检查是否获胜
     * @param row 行号
     * @param col 列号
     * @return 是否获胜
     */
    bool checkWin(int row, int col);

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
     * @brief 获胜连线结构体
     */
    struct WinLine {
        QPoint start;  ///< 连线起点（棋盘坐标）
        QPoint end;    ///< 连线终点（棋盘坐标）
        bool valid;    ///< 是否有效
        WinLine() : valid(false) {}
        WinLine(const QPoint& s, const QPoint& e) : start(s), end(e), valid(true) {}
    };

    std::vector<std::vector<PieceType>> board;  ///< 棋盘状态数组
    PieceType currentPlayer;                    ///< 当前玩家
    bool gameOver;                          ///< 游戏是否结束
    bool aiEnabled;                         ///< 是否启用AI
    std::unique_ptr<AIStrategy> aiStrategy;     ///< AI策略
    PieceType playerPieceType;                 ///< 玩家选择的棋子颜色
    
    int remainingUndos;                     ///< 剩余悔棋次数
    std::stack<Move> moveHistory;           ///< 移动历史记录

    QPoint lastMove;                        ///< 最后一个落子位置
    WinLine winLine;                        ///< 获胜连线

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
     * @brief 绘制最后落子标记
     * @param painter 画笔对象
     */
    void drawLastMove(QPainter &painter);

    /**
     * @brief 绘制获胜连线
     * @param painter 画笔对象
     */
    void drawWinLine(QPainter &painter);

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
    void showGameOver(PieceType winner);

    /**
     * @brief AI下棋
     */
    void makeAIMove();

    /**
     * @brief 执行悔棋操作
     * @return 是否成功悔棋
     */
    bool undoMove();

    /**
     * @brief 创建AI策略实例
     * @param strategyName 策略名称
     * @return AI策略实例
     */
    std::unique_ptr<AIStrategy> createAIStrategy(const QString& strategyName);

    /**
     * @brief 检查当前是否轮到AI下棋
     * @return 是否轮到AI下棋
     */
    bool isAITurn() const {
        return aiEnabled && currentPlayer != playerPieceType;
    }
};

#endif // BOARD_H 