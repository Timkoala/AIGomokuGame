#include "gamedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

GameDialog::GameDialog(QWidget *parent)
    : QDialog(parent)
    , gameMode(GameMode::PlayerVsPlayer)
    , aiDifficulty(3)
    , undoLimit(3)  // 默认允许3次悔棋
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
    
    // 创建悔棋次数设置
    QHBoxLayout *undoLayout = new QHBoxLayout;
    undoLabel = new QLabel("允许悔棋次数:", this);
    undoSpinBox = new QSpinBox(this);
    undoSpinBox->setRange(0, 10);  // 允许0-10次悔棋
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
    connect(buttonBox, &QDialogButtonBox::accepted, this, &GameDialog::onOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &GameDialog::reject);
    
    // 设置窗口属性
    setModal(true);
    setFixedSize(sizeHint());
}

void GameDialog::onGameModeChanged(int index)
{
    gameMode = static_cast<GameMode>(index);
    // 只在人机对战模式下启用难度设置
    difficultyLabel->setEnabled(gameMode == GameMode::PlayerVsAI);
    difficultySpinBox->setEnabled(gameMode == GameMode::PlayerVsAI);
}

void GameDialog::onOkClicked()
{
    aiDifficulty = difficultySpinBox->value();
    undoLimit = undoSpinBox->value();
    accept();
} 