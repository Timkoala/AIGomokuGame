#include "board.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTimer>
#include <chrono>

Board::Board(QWidget *parent)
    : QWidget(parent)
    , board(BOARD_SIZE, std::vector<Player>(BOARD_SIZE, Player::None))  // 初始化棋盘数组
    , currentPlayer(Player::Black)  // 黑方先手
    , gameOver(false)  // 初始化游戏状态
    , aiEnabled(false)
    , aiDifficulty(3)
    , rng(std::chrono::steady_clock::now().time_since_epoch().count())
    , remainingUndos(3)
    , lastMove(QPoint(-1, -1))
    , winLine()
{
    // 设置固定大小
    setFixedSize(BOARD_SIZE * CELL_SIZE + 2 * MARGIN,
                 BOARD_SIZE * CELL_SIZE + 2 * MARGIN);
    // 启用右键点击
    setContextMenuPolicy(Qt::PreventContextMenu);
}

void Board::resetGame(bool enableAI, int difficulty, int undoLimit)
{
    // 重置棋盘状态
    board = std::vector<std::vector<Player>>(
        BOARD_SIZE, std::vector<Player>(BOARD_SIZE, Player::None));
    currentPlayer = Player::Black;
    gameOver = false;
    aiEnabled = enableAI;
    aiDifficulty = difficulty;
    remainingUndos = undoLimit;
    
    // 清空移动历史
    while (!moveHistory.empty()) {
        moveHistory.pop();
    }
    
    // 重置最后落子位置和获胜连线
    lastMove = QPoint(-1, -1);
    winLine = WinLine();
    
    update();
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
            if (board[row][col] != Player::None) {
                // 计算棋子位置
                QPoint pos = boardToPixel(row, col);
                // 设置棋子颜色
                QColor color = (board[row][col] == Player::Black) ? Qt::black : Qt::white;
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
        board[row][col] == Player::None) {
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
        currentPlayer = (currentPlayer == Player::Black) ? Player::White : Player::Black;

        // 如果启用AI且当前是AI的回合
        if (aiEnabled && currentPlayer == Player::White) {
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
    board[lastMove.row][lastMove.col] = Player::None;
    currentPlayer = lastMove.player;

    // 减少剩余悔棋次数（仅在玩家回合减少）
    if (!aiEnabled || lastMove.player == Player::Black) {
        remainingUndos--;
    }

    return true;
}

void Board::makeAIMove()
{
    if (gameOver || !aiEnabled || currentPlayer != Player::White) {
        return;
    }

    // 获取所有可能的移动
    auto moves = getAvailableMoves();
    if (moves.empty()) {
        return;
    }

    // 根据难度选择最佳移动
    std::pair<int, int> bestMove = moves[0];
    int bestScore = -1;

    for (const auto &move : moves) {
        int score = evaluatePosition(move.first, move.second, Player::White);
        
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        } else if (score == bestScore && 
                  std::uniform_int_distribution<>(1, 5)(rng) > aiDifficulty) {
            bestMove = move;
        }
    }

    // 记录AI的移动
    moveHistory.push(Move(bestMove.first, bestMove.second, Player::White));

    // 执行移动
    board[bestMove.first][bestMove.second] = Player::White;
    
    if (checkWin(bestMove.first, bestMove.second)) {
        gameOver = true;
        showGameOver(Player::White);
    } else {
        currentPlayer = Player::Black;
    }
    
    update();
}

std::vector<std::pair<int, int>> Board::getAvailableMoves()
{
    std::vector<std::pair<int, int>> moves;
    moves.reserve(BOARD_SIZE * BOARD_SIZE);

    // 首先检查已有棋子周围的空位
    std::vector<std::vector<bool>> checked(BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));
    
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (board[row][col] != Player::None) {
                // 检查周围8个方向
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        if (dr == 0 && dc == 0) continue;
                        
                        int newRow = row + dr;
                        int newCol = col + dc;
                        
                        if (newRow >= 0 && newRow < BOARD_SIZE && 
                            newCol >= 0 && newCol < BOARD_SIZE &&
                            !checked[newRow][newCol] &&
                            board[newRow][newCol] == Player::None) {
                            moves.emplace_back(newRow, newCol);
                            checked[newRow][newCol] = true;
                        }
                    }
                }
            }
        }
    }

    // 如果是空棋盘或没有找到合适的位置，返回中心位置
    if (moves.empty()) {
        moves.emplace_back(BOARD_SIZE / 2, BOARD_SIZE / 2);
    }

    return moves;
}

int Board::evaluatePosition(int row, int col, Player player)
{
    int score = 0;
    const std::vector<std::pair<int, int>> directions = {
        {1, 0}, {0, 1}, {1, 1}, {1, -1}
    };

    // 临时放置棋子以评估位置
    board[row][col] = player;

    for (const auto &dir : directions) {
        int count = 1;
        bool blocked = false;

        // 向正方向检查
        for (int i = 1; i < 5; ++i) {
            int newRow = row + dir.first * i;
            int newCol = col + dir.second * i;
            if (newRow < 0 || newRow >= BOARD_SIZE || newCol < 0 || newCol >= BOARD_SIZE) {
                blocked = true;
                break;
            }
            if (board[newRow][newCol] != player) {
                if (board[newRow][newCol] != Player::None) blocked = true;
                break;
            }
            count++;
        }

        // 向反方向检查
        for (int i = 1; i < 5; ++i) {
            int newRow = row - dir.first * i;
            int newCol = col - dir.second * i;
            if (newRow < 0 || newRow >= BOARD_SIZE || newCol < 0 || newCol >= BOARD_SIZE) {
                blocked = true;
                break;
            }
            if (board[newRow][newCol] != player) {
                if (board[newRow][newCol] != Player::None) blocked = true;
                break;
            }
            count++;
        }

        // 根据连子数和是否被封堵计算分数
        if (count >= 5) score += 100000;
        else if (count == 4) score += blocked ? 1000 : 10000;
        else if (count == 3) score += blocked ? 100 : 1000;
        else if (count == 2) score += blocked ? 10 : 100;
        else score += blocked ? 1 : 10;
    }

    // 移除临时放置的棋子
    board[row][col] = Player::None;

    return score;
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
        Player current = board[row][col];  // 当前玩家
        
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

void Board::showGameOver(Player winner)
{
    // 显示游戏结束对话框
    QString message = (winner == Player::Black) ? "黑方胜利！" : "白方胜利！";
    QMessageBox::information(this, "游戏结束", message);
}

bool Board::saveGameState(const QString& filename)
{
    GameSave::SaveData data;
    
    // 保存基本信息
    data.timestamp = QDateTime::currentDateTime();
    data.isAIEnabled = aiEnabled;
    data.aiDifficulty = aiDifficulty;
    data.undoLimit = remainingUndos;  // 保存当前剩余次数作为限制
    data.remainingUndos = remainingUndos;
    data.currentPlayer = static_cast<int>(currentPlayer);
    
    // 保存棋盘状态
    data.board.resize(BOARD_SIZE, std::vector<int>(BOARD_SIZE));
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            data.board[i][j] = static_cast<int>(board[i][j]);
        }
    }
    
    // 保存移动历史
    std::stack<Move> tempHistory = moveHistory;
    while (!tempHistory.empty()) {
        const Move& move = tempHistory.top();
        data.history.insert(data.history.begin(), 
            GameSave::Move(move.row, move.col, move.player));  // 直接使用 Player 枚举
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
    
    // 加载基本信息
    aiEnabled = data.isAIEnabled;
    aiDifficulty = data.aiDifficulty;
    remainingUndos = data.remainingUndos;
    currentPlayer = static_cast<Player>(data.currentPlayer);
    gameOver = false;  // 重新加载时重置游戏结束状态
    
    // 加载棋盘状态
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = static_cast<Player>(data.board[i][j]);
        }
    }
    
    // 加载移动历史
    while (!moveHistory.empty()) {
        moveHistory.pop();
    }
    for (const auto& move : data.history) {
        moveHistory.push(Move(move.row, move.col, static_cast<Player>(move.player)));
    }
    
    update();  // 重绘棋盘
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