#pragma once

#include <string>
#include <memory>

#include "mcts_config.pb.h"

void LoadConfig(const char *config_path);
//void LoadConfig(const std::string &config_path);
void InitConfig(const std::string& config_path);