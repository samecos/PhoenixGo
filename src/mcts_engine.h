#pragma once

#include <atomic>
#include <vector>
#include <thread>
#include <future>

#include "go_comm.h"
#include "go_state.h"
#include "task_queue.h"
#include "wait_group.h"
#include "thread_conductor.h"
#include "timer.h"
#include "zero_model_base.h"

#include "train_data.h"
#include "mcts_config.h"
#include "mcts_monitor.h"
#include "mcts_debugger.h"
#include "byo_yomi_timer.h"
#include "config.h"

struct TreeNode
{
    std::atomic<TreeNode*> fa;
    std::atomic<TreeNode*> ch; // child nodes must allocate contiguously
    std::atomic<int> ch_len;
    std::atomic<int> size;
    std::atomic<int> expand_state;

    std::atomic<int> move;
    std::atomic<int> visit_count;
    std::atomic<int> virtual_loss_count;
    std::atomic<int64_t> total_action;
    std::atomic<float> prior_prob;
    std::atomic<float> value;
};

const int64_t k_action_value_base = 1 << 16;
const int k_unexpanded = 0;
const int k_expanding = 1;
const int k_expanded = 2;

//typedef std::function<void(int, std::vector<float>, float)> EvalCallback;

//struct EvalTask
//{
//    std::vector<bool> features;
//    EvalCallback callback;
//};

class MCTSEngine
{
 public:

    MCTSEngine(const MCTSConfig &config);
    MCTSEngine(const MCTSConfig& config, int id);
    ~MCTSEngine();


    void Init();
    void InitSelfplay(int single_thread);
    void analyzes();
    void Reset(const std::string &init_moves="");
    void Move(GoCoordId x, GoCoordId y);
    void GenMove(GoCoordId &x, GoCoordId &y);
    void GenMove(GoCoordId &x, GoCoordId &y, std::vector<int> &visit_count, float &v_resign);
    bool Undo();
    const GoState &GetBoard();
    MCTSConfig &GetConfig();
    void SetPendingConfig(std::unique_ptr<MCTSConfig> config);
    MCTSDebugger &GetDebugger();
    //int GetModelGlobalStep();
    ByoYomiTimer &GetByoYomiTimer();
    TreeNode* GetRoot()
    {
        return m_root;
    }
    void RunSelfplay(int single_thread);
    bool GetStatus()
    {
        return m_selfplay_is_over;
    }
    int GetEngineID()
    {
        return m_engine_id;
    }
    int SetEngineID(int id)
    {
        return m_engine_id = id;
    }
 private:
    TreeNode *InitNode(TreeNode *node, TreeNode *fa, int move, float prior_prob);
    TreeNode *FindChild(TreeNode *node, int move);

    void Eval(const GoState &board, EvalCallback callback);
    //void EvalRoutine(std::unique_ptr<ZeroModelBase> model);

    TreeNode *Select(GoState &board);
    TreeNode *SelectChild(TreeNode *node);
    int Expand(TreeNode *node, GoState &board, const std::vector<float> &policy);
    void Backup(TreeNode *node, float value, int ch_len);
    void UndoVirtualLoss(TreeNode *node);

    bool CheckEarlyStop(int64_t timeout_us);
    bool CheckUnstable();
    bool CheckBehind();

    int64_t GetSearchTimeoutUs();
    int64_t GetSearchOvertimeUs(int64_t timeout_us);

    void Search();
    void SearchWait(int64_t timeout_us, bool is_overtime);
    void SearchResume();
    void SearchPause();
    void SearchRoutine();

    void ChangeRoot(TreeNode *node);
    void InitRoot();

    void DeleteRoutine();
    int DeleteTree(TreeNode *node);

    int GetBestMove(float &v_resign);
    int GetSamplingMove(float temperature);
    std::vector<int> GetVisitCount(TreeNode *node);

    template<class T>
    void TransformFeatures(T &features, int mode, bool reverse = false);
    void TransformCoord(GoCoordId &x, GoCoordId &y, int mode, bool reverse = false);

    void ApplyTemperature(std::vector<float> &probs, float temperature);

    //void TTableUpdate(uint64_t hash, int64_t value);
    //void TTableSync(TreeNode *node);
    //void TTableClear();

    void EvalCacheInsert(uint64_t hash, const std::vector<float> policy, const float value);
    bool EvalCacheFind(uint64_t hash, EvalData& evl);

    bool IsPassDisable();
    void OutputAnalysis();
    bool RunOnce(TrainData& td);
    
    
 private:
    MCTSConfig m_config;
    std::unique_ptr<MCTSConfig> m_pending_config;

    TreeNode *m_root;
    GoState m_board;
    bool m_selfplay_is_over;


    std::vector<std::thread> m_search_threads;
    ThreadConductor m_search_threads_conductor;
    bool m_is_searching;

    std::thread m_delete_thread;
    std::thread m_selfplay_thread;
    //std::thread m_analyze_thread;
    TaskQueue<TreeNode*> m_delete_queue;

    std::atomic<int> m_simulation_counter;
    Timer m_search_timer;

    int m_num_moves;
    std::string m_moves_str;
    std::string m_show_moves_str;
    int m_gen_passes;
    int m_engine_id;

    ByoYomiTimer m_byo_yomi_timer;

    MCTSMonitor m_monitor;
    MCTSDebugger m_debugger;

    friend class MCTSMonitor;
    friend class MCTSDebugger;

};
