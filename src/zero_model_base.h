#pragma once

#include <vector>
#include <functional>

#include "errordef.h"

#include "model_config.pb.h"

class ZeroModelBase
{
 public:
    typedef std::function<void(int, std::vector<std::vector<float>>, std::vector<float>)> callback_t;

    virtual ~ZeroModelBase() {}

    virtual int Init(const ModelConfig &model_config) = 0;

    virtual int Forward(const std::vector<std::vector<bool>> &inputs,
                        std::vector<std::vector<float>> &policy, std::vector<float> &value) = 0;

    virtual void Forward(const std::vector<std::vector<bool>> &inputs, callback_t callback)
    {
        std::vector<std::vector<float>> policy;
        std::vector<float> value;
        int ret = Forward(inputs, policy, value);
        callback(ret, std::move(policy), std::move(value));
    }

    virtual int GetGlobalStep(int &global_step) = 0;

    virtual int RpcQueueSize() { return 0; }

    virtual void Wait() {}

    enum {
        INPUT_DIM  = 19 * 19 * 17,
        OUTPUT_DIM = 19 * 19 + 1,
    };
};
