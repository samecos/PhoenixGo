#pragma once
#include <gflags/gflags.h>
#include <vector>
#include <thread>
#include <functional>
#include "task_queue.h"
#include "wait_group.h"
#include "mcts_config.h"

DECLARE_bool(lizzie);

typedef std::function<void(int, std::vector<float>, float)> EvalCallback;
struct EvalTask
{
    std::vector<bool> features;
    EvalCallback callback;
};

extern TaskQueue<EvalTask> g_eval_task_queue;

extern std::vector<std::thread> g_eval_with_batch_threads;

extern WaitGroup g_eval_threads_init_wg;
extern WaitGroup g_eval_tasks_wg;

extern std::unique_ptr<MCTSConfig> g_config;
extern std::thread g_analyze_thread;