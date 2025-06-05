#include "gamedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

GameDialog::GameDialog(QWidget *parent)
    : QDialog(parent)
    , gameMode(GameMode::PlayerVsPlayer)
    , aiStrategy("RuleBased")
    , aiDifficulty(3)
    , undoLimit(3)  // 默认允许3次悔棋
    , playerPieceType(PieceType::BLACK)  // 默认玩家执黑
{
    setWindowTitle("游戏设置");
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建游戏模式选择
    QHBoxLayout *modeLayout = new QHBoxLayout;
    QLabel *modeLabel = new QLabel("游戏模式:", this);
    modeComboBox = new QComboBox(this);
    modeComboBox->addItem("双人对战");
    modeComboBox->addItem("人机对战");
    modeLayout->addWidget(modeLabel);
    modeLayout->addWidget(modeComboBox);
    mainLayout->addLayout(modeLayout);
    
    // 创建AI策略选择
    QHBoxLayout *strategyLayout = new QHBoxLayout;
    strategyLabel = new QLabel("AI策略:", this);
    strategyComboBox = new QComboBox(this);
    strategyComboBox->addItem("规则基础AI");  // RuleBased
    strategyComboBox->addItem("A*启发式搜索AI");  // AStar
    strategyLayout->addWidget(strategyLabel);
    strategyLayout->addWidget(strategyComboBox);
    mainLayout->addLayout(strategyLayout);
    
    // 创建AI难度设置
    QHBoxLayout *difficultyLayout = new QHBoxLayout;
    difficultyLabel = new QLabel("AI难度:", this);
    difficultySpinBox = new QSpinBox(this);
    difficultySpinBox->setRange(1, 5);
    difficultySpinBox->setValue(3);
    difficultySpinBox->setEnabled(false);
    difficultyLayout->addWidget(difficultyLabel);
    difficultyLayout->addWidget(difficultySpinBox);
    mainLayout->addLayout(difficultyLayout);

    // 创建玩家棋子颜色选择
    QHBoxLayout *colorLayout = new QHBoxLayout;
    colorLabel = new QLabel("玩家执子:", this);
    colorComboBox = new QComboBox(this);
    colorComboBox->addItem("执黑");
    colorComboBox->addItem("执白");
    colorComboBox->setEnabled(false);  // 默认禁用（双人模式下无需选择）
    colorLayout->addWidget(colorLabel);
    colorLayout->addWidget(colorComboBox);
    mainLayout->addLayout(colorLayout);
    
    // 创建悔棋次数设置
    QHBoxLayout *undoLayout = new QHBoxLayout;
    undoLabel = new QLabel("允许悔棋次数:", this);
    undoSpinBox = new QSpinBox(this);
    undoSpinBox->setRange(0, 10);
    undoSpinBox->setValue(3);      // 默认3次
    undoLayout->addWidget(undoLabel);
    undoLayout->addWidget(undoSpinBox);
    mainLayout->addLayout(undoLayout);
    
    // 创建按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal,
        this
    );
    mainLayout->addWidget(buttonBox);
    
    // 连接信号和槽
    connect(modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GameDialog::onGameModeChanged);
    connect(strategyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GameDialog::onAIStrategyChanged);
    connect(colorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GameDialog::onPlayerColorChanged);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &GameDialog::onOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &GameDialog::reject);
    
    // 初始化控件状态
    onGameModeChanged(modeComboBox->currentIndex());
    
    // 设置窗口属性
    setModal(true);
    setFixedSize(sizeHint());
}

void GameDialog::onGameModeChanged(int index)
{
    gameMode = static_cast<GameMode>(index);
    bool isAIMode = (gameMode == GameMode::PlayerVsAI);
    
    // 启用/禁用AI相关控件
    strategyLabel->setEnabled(isAIMode);
    strategyComboBox->setEnabled(isAIMode);
    difficultyLabel->setEnabled(isAIMode);
    difficultySpinBox->setEnabled(isAIMode);
    colorLabel->setEnabled(isAIMode);
    colorComboBox->setEnabled(isAIMode);
}

void GameDialog::onAIStrategyChanged(int index)
{
    // 根据选择的策略设置aiStrategy
    switch (index) {
        case 0:
            aiStrategy = "RuleBased";
            break;
        case 1:
            aiStrategy = "AStar";
            break;
        default:
            aiStrategy = "RuleBased";
            break;
    }
}

void GameDialog::onPlayerColorChanged(int index)
{
    // 设置玩家选择的棋子颜色
    playerPieceType = (index == 0) ? PieceType::BLACK : PieceType::WHITE;
}

void GameDialog::onOkClicked()
{
    aiDifficulty = difficultySpinBox->value();
    undoLimit = undoSpinBox->value();
    accept();
} 