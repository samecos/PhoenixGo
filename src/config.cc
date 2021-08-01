#include "config.h"

TaskQueue<EvalTask> g_eval_task_queue(0);

std::vector<std::thread> g_eval_with_batch_threads;

WaitGroup g_eval_threads_init_wg;
WaitGroup g_eval_tasks_wg;

std::unique_ptr<MCTSConfig> g_config = std::unique_ptr<MCTSConfig>(new MCTSConfig);

std::thread g_analyze_thread;