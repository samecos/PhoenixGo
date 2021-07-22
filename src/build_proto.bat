@echo on
C:\dev\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe --cpp_out=.\  .\dist_config.proto
C:\dev\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe --cpp_out=.\  .\dist_zero_model.proto
C:\dev\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe --cpp_out=.\  .\mcts_config.proto
C:\dev\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe --cpp_out=.\  .\model_config.proto
C:\dev\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe --grpc_out=.\  --plugin=protoc-gen-grpc=C:\dev\vcpkg\installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe   .\dist_zero_model.proto