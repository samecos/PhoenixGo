#pragma once
#include "config.h"

bool m_is_quit = false;
TaskQueue<EvalTask> m_eval_task_queue(0);
