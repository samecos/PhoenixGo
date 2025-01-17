#if GOOGLE_TENSORRT

#include <fstream>
#include <string>

#include <glog/logging.h>
#include <gflags/gflags.h>

#include "cuda_runtime_api.h"
#include "NvInfer.h"
#include "NvUffParser.h"

DEFINE_string(model_path, "", "Path of model.");
DEFINE_string(data_type, "FP32", "Data type to build, FP32 or FP16.");
DEFINE_int32(max_batch_size, 4, "Max size for input batch.");
DEFINE_int32(max_workspace_size, 1<<30, "Parameter to control memory allocation (in bytes).");
DEFINE_int32(calib_iterations, 10000, "Num of iterations to run while calibration.");
DEFINE_string(storage_address, "", "Address of wegostorage");
DEFINE_int32(gpu, 0, "Gpu used while building.");

using namespace nvinfer1;
using namespace nvuffparser;

class Logger : public ILogger
{
    void log(Severity severity, const char *msg) override
    {
        switch (severity) {
         case Severity::kINTERNAL_ERROR: LOG(ERROR) << msg; break;
         case Severity::kERROR: LOG(ERROR) << msg; break;
         case Severity::kWARNING: LOG(WARNING) << msg; break;
         case Severity::kINFO: LOG(INFO) << msg; break;
        }
    }
} g_logger;

int main(int argc, char *argv[])
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    std::string uff_path = FLAGS_model_path + ".uff";
    std::string calib_path = FLAGS_model_path + ".calib";
    std::string output_path = FLAGS_model_path + "." + FLAGS_data_type + ".PLAN";

    IUffParser *parser = createUffParser();
    parser = createUffParser();
    parser->registerInput("inputs", DimsCHW(17, 19, 19), UffInputOrder::kNCHW);
    parser->registerOutput("policy");
    parser->registerOutput("value");

    IBuilder *builder = createInferBuilder(g_logger);
    INetworkDefinition *network = builder->createNetwork();

    CHECK(parser->parse(uff_path.c_str(), *network, DataType::kFLOAT));

    builder->setMaxBatchSize(FLAGS_max_batch_size);
    builder->setMaxWorkspaceSize(FLAGS_max_workspace_size);
    builder->setHalf2Mode(FLAGS_data_type == "FP16");

    ICudaEngine *engine = builder->buildCudaEngine(*network);
    CHECK_NOTNULL(engine);

    IHostMemory *serialized_engine = engine->serialize();

    std::ofstream output(output_path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(serialized_engine->data()), serialized_engine->size());
}

#else // GOOGLE_TENSORRT

#include <stdio.h>

int main()
{
    fprintf(stderr, "TensorRT is not enable!\n");
    return -1;
}

#endif // GOOGLE_TENSORRT
