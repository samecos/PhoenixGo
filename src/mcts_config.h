#pragma once

#include <string>
#include <memory>

#include "mcts_config.pb.h"

std::unique_ptr<MCTSConfig> LoadConfig(const char *config_path);
std::unique_ptr<MCTSConfig> LoadConfig(const std::string &config_path);
