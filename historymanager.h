#pragma once
#include <QByteArray>

struct HistoryNode {
    QByteArray canvasState;
    HistoryNode* prev = nullptr;
    HistoryNode* next = nullptr;
};

class HistoryManager {
public:
    HistoryManager() = default;
    ~HistoryManager();
    void pushState(const QByteArray& state);
    QByteArray undo();
    QByteArray redo();
    bool canUndo() const;
    bool canRedo() const;

private:
    HistoryNode* current = nullptr;
    void clearNote();
};
