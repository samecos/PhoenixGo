#pragma once
#include <vector>
#include <functional>
#include "task_queue.h"


typedef std::function<void(int, std::vector<float>, float)> EvalCallback;
struct EvalTask
{
    std::vector<bool> features;
    EvalCallback callback;
};
extern bool m_is_quit;
extern TaskQueue<EvalTask> m_eval_task_queue;