#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentGameMode(GameDialog::GameMode::PlayerVsPlayer)
    , currentAIStrategy("RuleBased")
    , currentAIDifficulty(3)
    , currentUndoLimit(3)
    , currentPlayerPieceType(PieceType::BLACK)
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

    // 创建保存游戏按钮
    saveButton = new QPushButton("保存游戏", this);
    buttonLayout->addWidget(saveButton);

    // 创建加载游戏按钮
    loadButton = new QPushButton("加载游戏", this);
    buttonLayout->addWidget(loadButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 设置中央窗口部件
    setCentralWidget(centralWidget);
    
    // 连接信号和槽
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetGame);
    connect(newGameButton, &QPushButton::clicked, this, &MainWindow::newGame);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveGame);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadGame);
    
    // 设置窗口大小
    resize(600, 650);
    
    // 启动时显示游戏设置对话框
    newGame();
}

void MainWindow::resetGame()
{
    // 使用当前的游戏模式重置游戏
    board->resetGame(currentGameMode == GameDialog::GameMode::PlayerVsAI,
                    currentAIStrategy,
                    currentAIDifficulty,
                    currentUndoLimit,
                    currentPlayerPieceType);
}

void MainWindow::newGame()
{
    // 创建并显示游戏设置对话框
    GameDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // 保存设置
        currentGameMode = dialog.getGameMode();
        currentAIStrategy = dialog.getAIStrategy();
        currentAIDifficulty = dialog.getAIDifficulty();
        currentUndoLimit = dialog.getUndoLimit();
        currentPlayerPieceType = dialog.getPlayerPieceType();
        // 使用新的设置重置游戏
        resetGame();
    }
}

void MainWindow::saveGame()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "保存游戏",
        QString(),
        "五子棋存档 (*.gomoku);;所有文件 (*.*)"
    );
    
    if (filename.isEmpty()) {
        return;
    }
    
    // 如果用户没有指定扩展名，添加默认扩展名
    if (!filename.endsWith(".gomoku", Qt::CaseInsensitive)) {
        filename += ".gomoku";
    }
    
    if (board->saveGameState(filename)) {
        QMessageBox::information(this, "成功", "游戏已成功保存！");
    } else {
        QMessageBox::warning(this, "错误", "保存游戏失败！");
    }
}

void MainWindow::loadGame()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "加载游戏",
        QString(),
        "五子棋存档 (*.gomoku);;所有文件 (*.*)"
    );
    
    if (filename.isEmpty()) {
        return;
    }
    
    if (board->loadGameState(filename)) {
        QMessageBox::information(this, "成功", "游戏已成功加载！");
    } else {
        QMessageBox::warning(this, "错误", "加载游戏失败！");
    }
} 