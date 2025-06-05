#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include "board.h"
#include "gamedialog.h"

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

private:
    Board *board;              ///< 棋盘对象指针
    QPushButton *resetButton;  ///< 重新开始按钮指针
    QPushButton *newGameButton; ///< 新游戏按钮指针
    GameDialog::GameMode currentGameMode;  ///< 当前游戏模式
    int currentAIDifficulty;   ///< 当前AI难度
    int currentUndoLimit;      ///< 当前允许的悔棋次数
};

#endif // MAINWINDOW_H 