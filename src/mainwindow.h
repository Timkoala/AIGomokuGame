#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QString>
#include "board.h"
#include "gamedialog.h"
#include "game_types.h"

/**
 * @brief 主窗口类
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    MainWindow(QWidget *parent = nullptr);

private slots:
    /**
     * @brief 重置游戏槽函数
     */
    void resetGame();

    /**
     * @brief 新游戏槽函数
     */
    void newGame();

    /**
     * @brief 保存游戏槽函数
     */
    void saveGame();

    /**
     * @brief 加载游戏槽函数
     */
    void loadGame();

private:
    Board *board;              ///< 棋盘对象指针
    QPushButton *resetButton;  ///< 重新开始按钮指针
    QPushButton *newGameButton; ///< 新游戏按钮指针
    QPushButton *saveButton;    ///< 保存游戏按钮指针
    QPushButton *loadButton;    ///< 加载游戏按钮指针
    GameDialog::GameMode currentGameMode;  ///< 当前游戏模式
    QString currentAIStrategy;   ///< 当前AI策略
    int currentAIDifficulty;    ///< 当前AI难度
    int currentUndoLimit;       ///< 当前允许的悔棋次数
    PieceType currentPlayerPieceType; ///< 当前玩家选择的棋子颜色
};

#endif // MAINWINDOW_H 