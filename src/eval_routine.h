#pragma once
#include <memory>

#include "zero_model_base.h"
#include "mcts_monitor.h"
#include "config.h"


class Eval_Routine
{
public:
	Eval_Routine();
	//Eval_Routine(MCTSEngine& engine);
	~Eval_Routine();

	bool Init();
	bool InitSelfplay();


	void EvalRoutine_out(std::unique_ptr<ZeroModelBase> model, bool is_monitor);

private:
	//MCTSMonitor m_monitor;
	//std::atomic<int> m_model_global_step;
};