#include "eval_routine.h"

#include <glog/logging.h>

#include "timer.h"
#include "str_utils.h"
#include "trt_zero_model.h"
#include "dist_zero_model_client.h"
#include "async_dist_zero_model_client.h"
Eval_Routine::Eval_Routine(MCTSEngine& engine)
    : m_monitor(&engine)
{}

Eval_Routine::~Eval_Routine()
{}

bool Eval_Routine::Init()
{
    std::vector<int> gpu_list;
    
    for (const std::string& gpu : SplitStr(g_config->gpu_list(), ',')) {
        gpu_list.push_back(gpu.empty() ? 0 : std::stoi(gpu));
    }
    for (int i = 0; i < g_config->num_eval_threads(); ++i) {
        std::unique_ptr<ZeroModelBase> model;
        if (g_config->enable_dist()) {
            const auto& addr = g_config->dist_svr_addrs(i % g_config->dist_svr_addrs_size());
            if (g_config->enable_async()) {
                model.reset(new AsyncDistZeroModelClient(SplitStr(addr, ','), g_config->dist_config()));
            }
            else {
                model.reset(new DistZeroModelClient(addr, g_config->dist_config()));
            }
        }
        else if (g_config->model_config().enable_tensorrt()) {
            model.reset(new TrtZeroModel(gpu_list[i % gpu_list.size()]));
        }
        g_eval_threads_init_wg.Add();
        //g_eval_with_batch_threads.emplace_back(&MCTSEngine::EvalRoutine, this, std::move(model));

        g_eval_with_batch_threads.emplace_back(&Eval_Routine::EvalRoutine_out, this, std::move(model));
    }


    return true;
}

void Eval_Routine::EvalRoutine_out(std::unique_ptr<ZeroModelBase> model)
{
    int ret = model->Init(g_config->model_config());
    CHECK_EQ(ret, 0) << "EvalRoutine: model init failed, ret " << ret;

    //int global_step;
    //ret = model->GetGlobalStep(global_step);
    //CHECK_EQ(ret, 0) << "EvalRoutine: model get global_step failed, ret " << ret;

    //LOG(INFO) << "EvalRoutine: init model done, global_step=" << global_step;
    //int expect_zero = 0;
    //if (!m_model_global_step.compare_exchange_strong(expect_zero, global_step)) {
    //    CHECK_EQ(expect_zero, global_step) << "EvalRoutine: global_step different with other routines";
    //}

    g_eval_threads_init_wg.Done();
    for (;;) {
        model->Wait();

        EvalTask task;
        std::vector<std::vector<bool>> inputs;
        std::vector<EvalCallback> callbacks;
        for (int i = 0; i < g_config->eval_batch_size(); ++i) {
            if (g_eval_task_queue.Pop(task, i ? g_config->eval_wait_batch_timeout_us() : -1)) {
                inputs.push_back(std::move(task.features));
                callbacks.push_back(std::move(task.callback));
            }
            else if (g_eval_task_queue.IsClose()) {
                LOG(WARNING) << "EvalRoutine: terminate";
                return; // terminate
            }
            else { // timeout
                break;
            }
        }

        size_t batch_size = inputs.size();
        m_monitor.MonEvalBatchSize(batch_size);

        Timer timer;
        model->Forward(
            inputs,
            [this, inputs, callbacks, batch_size, timer]
        (int ret, std::vector<std::vector<float>> policy, std::vector<float> value) {
                m_monitor.MonEvalCostMsPerBatch(timer.fms());

                // fill result
                if (ret == ERR_FORWARD_TIMEOUT) {
                    m_monitor.IncEvalTimeout();
                    for (size_t i = 0; i < batch_size; ++i) {
                        g_eval_task_queue.PushFront(EvalTask{ std::move(inputs[i]), std::move(callbacks[i]) });
                    }
                }
                else if (ret) {
                    LOG(ERROR) << "EvalRoutine: feed model failed, ret " << ret;
                    for (size_t i = 0; i < batch_size; ++i) {
                        callbacks[i](ret, {}, 0.0);
                    }
                }
                else {
                    CHECK_EQ(batch_size, policy.size())
                        << "EvalRoutine: batch size unmatch, expect " << batch_size << ", got" << policy.size();
                    CHECK_EQ(batch_size, value.size())
                        << "EvalRoutine: batch size unmatch, expect " << batch_size << ", got" << policy.size();
                    for (size_t i = 0; i < batch_size; ++i) {
                        callbacks[i](ret, std::move(policy[i]), value[i]);
                    }
                }
            }
        );

        if (g_config->enable_async()) {
            m_monitor.MonRpcQueueSize(model->RpcQueueSize());
        }
    }
}