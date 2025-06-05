#include "board.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>

Board::Board(QWidget *parent)
    : QWidget(parent)
    , board(BOARD_SIZE, std::vector<Player>(BOARD_SIZE, Player::None))  // 初始化棋盘数组
    , currentPlayer(Player::Black)  // 黑方先手
    , gameOver(false)  // 初始化游戏状态
{
    // 设置固定大小
    setFixedSize(BOARD_SIZE * CELL_SIZE + 2 * MARGIN,
                 BOARD_SIZE * CELL_SIZE + 2 * MARGIN);
}

void Board::resetGame()
{
    // 清空棋盘
    for (auto &row : board) {
        std::fill(row.begin(), row.end(), Player::None);
    }
    // 重置游戏状态
    currentPlayer = Player::Black;
    gameOver = false;
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
    if (gameOver) {
        return;
    }

    // 将鼠标坐标转换为棋盘坐标
    QPoint pos = pixelToBoard(event->pos().x(), event->pos().y());
    int row = pos.x();
    int col = pos.y();

    // 检查坐标是否有效且该位置为空
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE &&
        board[row][col] == Player::None) {
        // 放置棋子
        board[row][col] = currentPlayer;
        
        // 检查是否获胜
        if (checkWin(row, col)) {
            gameOver = true;
            showGameOver(currentPlayer);
        } else {
            // 切换当前玩家
            currentPlayer = (currentPlayer == Player::Black) ? Player::White : Player::Black;
        }
        
        // 重绘棋盘
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