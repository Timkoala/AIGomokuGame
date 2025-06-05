#include "board.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTimer>
#include <chrono>
#include "rule_based_ai.h"

Board::Board(QWidget *parent)
    : QWidget(parent)
    , board(BOARD_SIZE, std::vector<PieceType>(BOARD_SIZE, PieceType::None))
    , currentPlayer(PieceType::Black)
    , gameOver(false)
    , aiEnabled(false)
    , aiStrategy(nullptr)
    , remainingUndos(3)
    , lastMove(QPoint(-1, -1))
    , winLine()
{
    setFixedSize(BOARD_SIZE * CELL_SIZE + 2 * MARGIN,
                 BOARD_SIZE * CELL_SIZE + 2 * MARGIN);
    setContextMenuPolicy(Qt::PreventContextMenu);
}

void Board::resetGame(bool enableAI, const QString& aiStrategy, int difficulty, int undoLimit)
{
    board = std::vector<std::vector<PieceType>>(
        BOARD_SIZE, std::vector<PieceType>(BOARD_SIZE, PieceType::None));
    currentPlayer = PieceType::Black;
    gameOver = false;
    aiEnabled = enableAI;
    remainingUndos = undoLimit;
    
    if (enableAI) {
        setAIStrategy(aiStrategy);
        this->aiStrategy->setDifficulty(difficulty);
    } else {
        this->aiStrategy.reset();
    }
    
    while (!moveHistory.empty()) {
        moveHistory.pop();
    }
    
    lastMove = QPoint(-1, -1);
    winLine = WinLine();
    
    update();
}

void Board::setAIStrategy(const QString& strategyName)
{
    aiStrategy = createAIStrategy(strategyName);
}

std::unique_ptr<AIStrategy> Board::createAIStrategy(const QString& strategyName)
{
    if (strategyName == "RuleBased") {
        return std::make_unique<RuleBasedAI>();
    }
    // 在这里添加其他AI策略的创建
    return std::make_unique<RuleBasedAI>();  // 默认使用规则基础AI
}

void Board::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 启用抗锯齿

    drawBoard(painter);
    drawPieces(painter);
    drawLastMove(painter);
    if (gameOver && winLine.valid) {
        drawWinLine(painter);
    }
}

void Board::drawBoard(QPainter &painter)
{
    // 绘制棋盘背景（木色）
    painter.fillRect(rect(), QColor(240, 200, 150));
    
    // 设置画笔属性
    QPen pen(Qt::black, 1);
    painter.setPen(pen);
    
    // 绘制网格线
    for (int i = 0; i < BOARD_SIZE; ++i) {
        // 绘制横线
        painter.drawLine(MARGIN, MARGIN + i * CELL_SIZE,
                        MARGIN + (BOARD_SIZE - 1) * CELL_SIZE,
                        MARGIN + i * CELL_SIZE);
        // 绘制竖线
        painter.drawLine(MARGIN + i * CELL_SIZE, MARGIN,
                        MARGIN + i * CELL_SIZE,
                        MARGIN + (BOARD_SIZE - 1) * CELL_SIZE);
    }
}

void Board::drawPieces(QPainter &painter)
{
    // 遍历棋盘，绘制所有棋子
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (board[row][col] != PieceType::None) {
                // 计算棋子位置
                QPoint pos = boardToPixel(row, col);
                // 设置棋子颜色
                QColor color = (board[row][col] == PieceType::Black) ? Qt::black : Qt::white;
                painter.setPen(Qt::black);
                painter.setBrush(color);
                // 绘制棋子（圆形）
                painter.drawEllipse(pos, CELL_SIZE / 2 - 2, CELL_SIZE / 2 - 2);
            }
        }
    }
}

void Board::mousePressEvent(QMouseEvent *event)
{
    if (gameOver) return;

    if (event->button() == Qt::RightButton) {
        // 右键悔棋
        if (undoMove()) {
            update();
        }
        return;
    }

    QPoint boardPos = pixelToBoard(event->x(), event->y());
    int row = boardPos.x();
    int col = boardPos.y();

    // 检查是否在有效范围内且该位置为空
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE &&
        board[row][col] == PieceType::None) {
        // 记录移动
        moveHistory.push(Move(row, col, currentPlayer));
        board[row][col] = currentPlayer;
        lastMove = QPoint(row, col);  // 记录最后落子位置

        // 检查是否获胜
        if (checkWin(row, col)) {
            gameOver = true;
            showGameOver(currentPlayer);
            update();
            return;
        }

        // 切换玩家
        currentPlayer = (currentPlayer == PieceType::Black) ? PieceType::White : PieceType::Black;

        // 如果启用AI且当前是AI的回合
        if (aiEnabled && currentPlayer == PieceType::White) {
            update();
            QTimer::singleShot(100, this, &Board::makeAIMove);
        }

        update();
    }
}

bool Board::undoMove()
{
    if (moveHistory.empty() || gameOver) {
        return false;
    }

    // 获取最后一步移动
    Move lastMove = moveHistory.top();
    moveHistory.pop();

    // 恢复棋盘状态
    board[lastMove.row][lastMove.col] = PieceType::None;
    currentPlayer = lastMove.player;

    // 减少剩余悔棋次数（仅在玩家回合减少）
    if (!aiEnabled || lastMove.player == PieceType::Black) {
        remainingUndos--;
    }

    return true;
}

void Board::makeAIMove()
{
    if (gameOver || !aiEnabled || !aiStrategy || currentPlayer != PieceType::White) {
        return;
    }

    QPoint move = aiStrategy->getNextMove(board, currentPlayer);
    if (move.x() >= 0 && move.x() < BOARD_SIZE && 
        move.y() >= 0 && move.y() < BOARD_SIZE) {
        
        moveHistory.push(Move(move.x(), move.y(), currentPlayer));
        board[move.x()][move.y()] = currentPlayer;
        lastMove = move;
        
        if (checkWin(move.x(), move.y())) {
            gameOver = true;
            showGameOver(currentPlayer);
        } else {
            currentPlayer = PieceType::Black;
        }
        
        update();
    }
}

bool Board::checkWin(int row, int col)
{
    // 定义四个方向：垂直、水平、对角线、反对角线
    const std::vector<std::pair<int, int>> directions = {
        {1, 0},   // 垂直
        {0, 1},   // 水平
        {1, 1},   // 对角线
        {1, -1}   // 反对角线
    };

    // 检查每个方向
    for (const auto &dir : directions) {
        int count = 1;  // 当前方向的连续棋子数
        PieceType current = board[row][col];  // 当前玩家
        
        QPoint start(row, col);
        QPoint end(row, col);

        // 向正方向检查
        for (int i = 1; i < 5; ++i) {
            int newRow = row + dir.first * i;
            int newCol = col + dir.second * i;
            // 检查边界和是否为同色棋子
            if (newRow < 0 || newRow >= BOARD_SIZE || newCol < 0 || newCol >= BOARD_SIZE ||
                board[newRow][newCol] != current) {
                break;
            }
            count++;
            end = QPoint(newRow, newCol);
        }

        // 向反方向检查
        for (int i = 1; i < 5; ++i) {
            int newRow = row - dir.first * i;
            int newCol = col - dir.second * i;
            // 检查边界和是否为同色棋子
            if (newRow < 0 || newRow >= BOARD_SIZE || newCol < 0 || newCol >= BOARD_SIZE ||
                board[newRow][newCol] != current) {
                break;
            }
            count++;
            start = QPoint(newRow, newCol);
        }

        // 如果任意方向达到5个连续棋子，则获胜
        if (count >= 5) {
            winLine = WinLine(start, end);  // 记录获胜连线
            return true;
        }
    }

    return false;
}

QPoint Board::boardToPixel(int row, int col) const
{
    // 将棋盘坐标转换为像素坐标
    return QPoint(MARGIN + col * CELL_SIZE, MARGIN + row * CELL_SIZE);
}

QPoint Board::pixelToBoard(int x, int y) const
{
    // 将像素坐标转换为棋盘坐标
    int row = (y - MARGIN + CELL_SIZE / 2) / CELL_SIZE;
    int col = (x - MARGIN + CELL_SIZE / 2) / CELL_SIZE;
    return QPoint(row, col);
}

void Board::showGameOver(PieceType winner)
{
    QString message = (winner == PieceType::Black) ? "黑方胜利！" : "白方胜利！";
    QMessageBox::information(this, "游戏结束", message);
}

bool Board::saveGameState(const QString& filename)
{
    GameSave::SaveData data;
    
    data.timestamp = QDateTime::currentDateTime();
    data.isAIEnabled = aiEnabled;
    data.aiDifficulty = aiStrategy ? aiStrategy->getDifficulty() : 1;
    data.undoLimit = remainingUndos;
    data.remainingUndos = remainingUndos;
    data.currentPlayer = static_cast<int>(currentPlayer);
    
    data.board.resize(BOARD_SIZE, std::vector<int>(BOARD_SIZE));
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            data.board[i][j] = static_cast<int>(board[i][j]);
        }
    }
    
    std::stack<Move> tempHistory = moveHistory;
    while (!tempHistory.empty()) {
        const Move& move = tempHistory.top();
        data.history.insert(data.history.begin(), 
            GameSave::Move(move.row, move.col, move.player));
        tempHistory.pop();
    }
    
    return GameSave::saveGame(filename, data);
}

bool Board::loadGameState(const QString& filename)
{
    GameSave::SaveData data;
    if (!GameSave::loadGame(filename, data)) {
        return false;
    }
    
    aiEnabled = data.isAIEnabled;
    if (aiEnabled) {
        setAIStrategy("RuleBased");  // 默认使用规则基础AI
        aiStrategy->setDifficulty(data.aiDifficulty);
    }
    remainingUndos = data.remainingUndos;
    currentPlayer = static_cast<PieceType>(data.currentPlayer);
    gameOver = false;
    
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = static_cast<PieceType>(data.board[i][j]);
        }
    }
    
    while (!moveHistory.empty()) {
        moveHistory.pop();
    }
    for (const auto& move : data.history) {
        moveHistory.push(Move(move.row, move.col, move.player));
    }
    
    update();
    return true;
}

void Board::drawLastMove(QPainter &painter)
{
    if (!moveHistory.empty()) {
        QPoint pixelPos = boardToPixel(lastMove.x(), lastMove.y());
        
        // 设置画笔
        QPen pen(Qt::red);
        pen.setWidth(2);
        painter.setPen(pen);
        
        // 绘制一个小方框标记最后落子位置
        const int markSize = 6;
        painter.drawLine(pixelPos.x() - markSize, pixelPos.y() - markSize,
                        pixelPos.x() + markSize, pixelPos.y() - markSize);
        painter.drawLine(pixelPos.x() + markSize, pixelPos.y() - markSize,
                        pixelPos.x() + markSize, pixelPos.y() + markSize);
        painter.drawLine(pixelPos.x() + markSize, pixelPos.y() + markSize,
                        pixelPos.x() - markSize, pixelPos.y() + markSize);
        painter.drawLine(pixelPos.x() - markSize, pixelPos.y() + markSize,
                        pixelPos.x() - markSize, pixelPos.y() - markSize);
    }
}

void Board::drawWinLine(QPainter &painter)
{
    // 设置画笔
    QPen pen(Qt::red);
    pen.setWidth(3);
    painter.setPen(pen);

    // 获取像素坐标
    QPoint start = boardToPixel(winLine.start.x(), winLine.start.y());
    QPoint end = boardToPixel(winLine.end.x(), winLine.end.y());

    // 绘制连线
    painter.drawLine(start, end);
} 