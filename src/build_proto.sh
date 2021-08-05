protoc -I=/home/maluki/PhoenixGo/src --cpp_out=/home/maluki/PhoenixGo/src/  dist_config.proto
protoc -I=/home/maluki/PhoenixGo/src --cpp_out=/home/maluki/PhoenixGo/src/  dist_zero_model.proto
protoc -I=/home/maluki/PhoenixGo/src --cpp_out=/home/maluki/PhoenixGo/src/  mcts_config.proto
protoc -I=/home/maluki/PhoenixGo/src --cpp_out=/home/maluki/PhoenixGo/src/  model_config.proto
protoc -I=/home/maluki/PhoenixGo/src --grpc_out=/home/maluki/PhoenixGo/src/  --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin dist_zero_model.proto