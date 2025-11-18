#include "historymanager.h"

void HistoryManager::pushState(const QByteArray& state) {
    auto* node = new HistoryNode{state};
    if (current) {
        // 清除 redo 分支
        HistoryNode* temp = current->next;
        while (temp) {
            HistoryNode* next = temp->next;
            delete temp;
            temp = next;
        }
        current->next = node;
        node->prev = current;
    }
    current = node;
}

QByteArray HistoryManager::undo() {
    if (current && current->prev) {
        current = current->prev;
        return current->canvasState;
    }
    return QByteArray();
}

QByteArray HistoryManager::redo() {
    if (current && current->next) {
        current = current->next;
        return current->canvasState;
    }
    return QByteArray();
}

bool HistoryManager::canUndo() const {
    return current && current->prev;
}

bool HistoryManager::canRedo() const {
    return current && current->next;
}

HistoryManager::~HistoryManager() {
    //historymanager.cpp:43:17: Definition of implicitly declared destructor
    // 往最前面走
    while (current && current->prev) {
        current = current->prev;
    }
    // 從頭開始刪掉所有節點
    while (current) {
        HistoryNode* next = current->next;
        delete current;
        current = next;
    }
}
