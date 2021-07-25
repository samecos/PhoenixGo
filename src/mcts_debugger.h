#pragma once

#include <string>

class MCTSEngine;
class MCTSDebugger
{
 public:
    MCTSDebugger(MCTSEngine *engine);

    void Debug(); // call before move

    std::string GetDebugStr();
    std::string GetLastMoveDebugStr(); // call after move
    void UpdateLastMoveDebugStr();

    std::string GetMainMovePath(int rank);
    std::string GetMainMovePaths(int rank);
    void PrintTree(int depth, int topk, const std::string &prefix = "");
    MCTSEngine* GetEngine() {
        return m_engine;
    }
 private:
    MCTSEngine *m_engine;
    std::string m_last_move_debug_str;
};
