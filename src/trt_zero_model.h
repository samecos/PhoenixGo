#pragma once

#include <memory>

#include "zero_model_base.h"
#include "model_config.pb.h"

namespace nvinfer1 {
    class ICudaEngine;
    class IRuntime;
    class IExecutionContext;
}

class TrtZeroModel final : public ZeroModelBase
{
 public:
    TrtZeroModel(int gpu);
    ~TrtZeroModel();

    int Init(const ModelConfig &model_config) override;

    // input  [batch, 19 * 19 * 17]
    // policy [batch, 19 * 19 + 1]
    int Forward(const std::vector<std::vector<bool>> &inputs,
                std::vector<std::vector<float>> &policy, std::vector<float> &value) override;

    int GetGlobalStep(int &global_step) override;

 private:
    nvinfer1::ICudaEngine *m_engine;
    nvinfer1::IRuntime *m_runtime;
    nvinfer1::IExecutionContext *m_context;
    std::vector<void*> m_cuda_buf;
    int m_gpu;
    int m_global_step;
};
