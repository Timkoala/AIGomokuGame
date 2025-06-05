#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentGameMode(GameDialog::GameMode::PlayerVsPlayer)
    , currentAIDifficulty(3)
{
    // 设置窗口标题
    setWindowTitle("五子棋");
    
    // 创建中央窗口部件和布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建棋盘对象并添加到布局中
    board = new Board(this);
    mainLayout->addWidget(board);
    
    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    // 创建新游戏按钮
    newGameButton = new QPushButton("新游戏", this);
    buttonLayout->addWidget(newGameButton);
    
    // 创建重新开始按钮
    resetButton = new QPushButton("重新开始", this);
    buttonLayout->addWidget(resetButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 设置中央窗口部件
    setCentralWidget(centralWidget);
    
    // 连接信号和槽
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetGame);
    connect(newGameButton, &QPushButton::clicked, this, &MainWindow::newGame);
    
    // 设置窗口大小
    resize(600, 650);
    
    // 启动时显示游戏设置对话框
    newGame();
}

void MainWindow::resetGame()
{
    // 使用当前的游戏模式重置游戏
    board->resetGame(currentGameMode == GameDialog::GameMode::PlayerVsAI, currentAIDifficulty);
}

void MainWindow::newGame()
{
    // 创建并显示游戏设置对话框
    GameDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // 保存设置
        currentGameMode = dialog.getGameMode();
        currentAIDifficulty = dialog.getAIDifficulty();
        // 使用新的设置重置游戏
        resetGame();
    }
} 