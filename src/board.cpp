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
{
    // 设置固定大小
    setFixedSize(BOARD_SIZE * CELL_SIZE + 2 * MARGIN,
                 BOARD_SIZE * CELL_SIZE + 2 * MARGIN);
    // 启用右键点击
    setContextMenuPolicy(Qt::PreventContextMenu);
}

void Board::resetGame(bool enableAI, int difficulty, int undoLimit)
{
    // 清空棋盘
    for (auto &row : board) {
        std::fill(row.begin(), row.end(), Player::None);
    }
    // 重置游戏状态
    currentPlayer = Player::Black;
    gameOver = false;
    aiEnabled = enableAI;
    aiDifficulty = difficulty;
    remainingUndos = undoLimit;
    
    // 清空移动历史
    while (!moveHistory.empty()) {
        moveHistory.pop();
    }
    
    // 重绘棋盘
    update();
}

void Board::paintEvent(QPaintEvent *)
{
    // 创建画笔对象
    QPainter painter(this);
    // 启用抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制棋盘和棋子
    drawBoard(painter);
    drawPieces(painter);
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
    // 如果游戏已结束，不处理鼠标事件
    if (gameOver || (aiEnabled && currentPlayer == Player::White)) {
        return;
    }

    // 处理右键点击（悔棋）
    if (event->button() == Qt::RightButton) {
        if (remainingUndos > 0 && !moveHistory.empty()) {
            if (undoMove()) {
                // 如果是人机对战，需要撤销两步（玩家和AI的移动）
                if (aiEnabled && !moveHistory.empty()) {
                    undoMove();
                }
                update();
            }
        } else if (remainingUndos <= 0) {
            QMessageBox::information(this, "提示", "已无悔棋机会");
        }
        return;
    }

    // 处理左键点击（落子）
    if (event->button() == Qt::LeftButton) {
        QPoint pos = pixelToBoard(event->pos().x(), event->pos().y());
        int row = pos.x();
        int col = pos.y();

        if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE &&
            board[row][col] == Player::None) {
            // 记录移动
            moveHistory.push(Move(row, col, currentPlayer));
            
            // 放置棋子
            board[row][col] = currentPlayer;
            
            if (checkWin(row, col)) {
                gameOver = true;
                showGameOver(currentPlayer);
            } else {
                currentPlayer = (currentPlayer == Player::Black) ? Player::White : Player::Black;
                
                // 如果启用了AI且轮到AI下棋
                if (aiEnabled && currentPlayer == Player::White && !gameOver) {
                    update();
                    QTimer::singleShot(500, this, &Board::makeAIMove);
                }
            }
            
            update();
        }
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
        }

        // 如果任意方向达到5个连续棋子，则获胜
        if (count >= 5) {
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