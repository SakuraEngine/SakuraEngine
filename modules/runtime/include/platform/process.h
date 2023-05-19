#pragma once
#include "platform/configure.h"

typedef struct SProcess SProcess;
typedef struct SProcess* SProcessHandle;
typedef uint32_t SProcessId;

RUNTIME_EXTERN_C RUNTIME_API
SProcessHandle skr_run_process(const char8_t* command, const char8_t** arguments, uint32_t arg_count, const char8_t* stdout_file);

RUNTIME_EXTERN_C RUNTIME_API
SProcessId skr_get_current_process_id();

RUNTIME_EXTERN_C RUNTIME_API
SProcessId skr_get_process_id(SProcessHandle);

RUNTIME_EXTERN_C RUNTIME_API
int skr_wait_process(SProcessHandle process);