#include "mcts_config.h"

#include <fstream>

#include <gflags/gflags.h>
#include <google/protobuf/text_format.h>
#include <glog/logging.h>
#include "config.h"
DECLARE_string(gpu_list);

void LoadConfig(const char *config_path)
{
    g_config.reset(new MCTSConfig);
    //auto config = std::unique_ptr<MCTSConfig>(new MCTSConfig);
    std::ostringstream conf_ss;
    if (!(conf_ss << std::ifstream(config_path).rdbuf())) {
        PLOG(ERROR) << "read config file " << config_path << " error";
        return ;
    }
    if (!google::protobuf::TextFormat::ParseFromString(conf_ss.str(), g_config.get())) {
        LOG(ERROR) << "parse config file " << config_path << " error! buf=" << conf_ss.str();
        return ;
    }
    //return config;
}

 void LoadConfig(const std::string &config_path)
{
    LoadConfig(config_path.c_str());
}

void InitConfig(const std::string& config_path)
{
    LoadConfig(config_path);
    if (g_config == nullptr) {
        return ;
    }

    if (FLAGS_gpu_list.size()) {
        g_config->set_gpu_list(FLAGS_gpu_list);
    }
}

