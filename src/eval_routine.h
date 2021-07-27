#pragma once
#include <memory>

#include "zero_model_base.h"
#include "mcts_monitor.h"
#include "config.h"


class Eval_Routine
{
public:
	Eval_Routine(MCTSEngine* engine);
	~Eval_Routine();

	void EvalRoutine_out(std::unique_ptr<ZeroModelBase> model);

private:
	MCTSMonitor m_monitor;
	std::atomic<int> m_model_global_step;
};