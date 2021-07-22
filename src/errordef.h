#pragma once

enum {
    ERR_INVALID_INPUT        = -1,
    ERR_FORWARD_TIMEOUT      = -2,

    ERR_READ_CHECKPOINT      = -1000,
    ERR_CREATE_SESSION       = -1001,
    ERR_CREATE_GRAPH         = -1002,
    ERR_RESTORE_VAR          = -1003,
    ERR_SESSION_RUN          = -1005,

    // tensorrt error
    ERR_READ_TRT_MODEL       = -2000,
    ERR_LOAD_TRT_ENGINE      = -2001,
    ERR_CUDA_MALLOC          = -2002,
    ERR_CUDA_FREE            = -2003,
    ERR_CUDA_MEMCPY          = -2004,

    // rpc error
    ERR_GLOBAL_STEP_CONFLICT = -3000,
    ERR_EMPTY_RESP           = -3001,
};
