#ifndef GAMEDIALOG_H
#define GAMEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>

/**
 * @brief 游戏设置对话框类
 */
class GameDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief 游戏模式枚举
     */
    enum class GameMode {
        PlayerVsPlayer,    ///< 双人对战
        PlayerVsAI        ///< 人机对战
    };

    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit GameDialog(QWidget *parent = nullptr);

    /**
     * @brief 获取选择的游戏模式
     */
    GameMode getGameMode() const { return gameMode; }

    /**
     * @brief 获取AI难度等级
     */
    int getAIDifficulty() const { return aiDifficulty; }

    /**
     * @brief 获取允许的悔棋次数
     */
    int getUndoLimit() const { return undoLimit; }

private slots:
    /**
     * @brief 游戏模式改变时的处理函数
     */
    void onGameModeChanged(int index);

    /**
     * @brief 确认按钮点击处理函数
     */
    void onOkClicked();

private:
    GameMode gameMode;        ///< 当前选择的游戏模式
    int aiDifficulty;        ///< AI难度等级（1-5）
    int undoLimit;           ///< 允许的悔棋次数
    
    QComboBox *modeComboBox;  ///< 游戏模式选择框
    QLabel *difficultyLabel;  ///< AI难度标签
    QSpinBox *difficultySpinBox;  ///< AI难度选择框
    QLabel *undoLabel;        ///< 悔棋次数标签
    QSpinBox *undoSpinBox;    ///< 悔棋次数选择框
};

#endif // GAMEDIALOG_H 