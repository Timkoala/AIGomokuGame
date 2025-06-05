#include "gamesave.h"
#include "board.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

bool GameSave::saveGame(const QString& filename, const SaveData& data)
{
    QJsonObject saveObj;
    
    // 保存基本信息
    saveObj["timestamp"] = data.timestamp.toString(Qt::ISODate);
    saveObj["isAIEnabled"] = data.isAIEnabled;
    saveObj["aiDifficulty"] = data.aiDifficulty;
    saveObj["undoLimit"] = data.undoLimit;
    saveObj["remainingUndos"] = data.remainingUndos;
    saveObj["currentPlayer"] = data.currentPlayer;
    
    // 保存棋盘状态
    QJsonArray boardArray;
    for (const auto& row : data.board) {
        QJsonArray rowArray;
        for (int cell : row) {
            rowArray.append(cell);
        }
        boardArray.append(rowArray);
    }
    saveObj["board"] = boardArray;
    
    // 保存移动历史
    QJsonArray historyArray;
    for (const auto& move : data.history) {
        QJsonObject moveObj;
        moveObj["row"] = move.row;
        moveObj["col"] = move.col;
        moveObj["player"] = static_cast<int>(move.player);
        historyArray.append(moveObj);
    }
    saveObj["history"] = historyArray;
    
    // 写入文件
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(saveObj);
    file.write(doc.toJson());
    return true;
}

bool GameSave::loadGame(const QString& filename, SaveData& data)
{
    // 读取文件
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        return false;
    }
    
    QJsonObject saveObj = doc.object();
    
    // 加载基本信息
    data.timestamp = QDateTime::fromString(saveObj["timestamp"].toString(), Qt::ISODate);
    data.isAIEnabled = saveObj["isAIEnabled"].toBool();
    data.aiDifficulty = saveObj["aiDifficulty"].toInt();
    data.undoLimit = saveObj["undoLimit"].toInt();
    data.remainingUndos = saveObj["remainingUndos"].toInt();
    data.currentPlayer = saveObj["currentPlayer"].toInt();
    
    // 加载棋盘状态
    QJsonArray boardArray = saveObj["board"].toArray();
    data.board.clear();
    data.board.reserve(boardArray.size());
    for (const auto& rowVal : boardArray) {
        QJsonArray rowArray = rowVal.toArray();
        std::vector<int> row;
        row.reserve(rowArray.size());
        for (const auto& cellVal : rowArray) {
            row.push_back(cellVal.toInt());
        }
        data.board.push_back(row);
    }
    
    // 加载移动历史
    QJsonArray historyArray = saveObj["history"].toArray();
    data.history.clear();
    data.history.reserve(historyArray.size());
    for (const auto& moveVal : historyArray) {
        QJsonObject moveObj = moveVal.toObject();
        data.history.emplace_back(
            moveObj["row"].toInt(),
            moveObj["col"].toInt(),
            static_cast<Player>(moveObj["player"].toInt())
        );
    }
    
    return true;
} 