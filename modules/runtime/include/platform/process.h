#pragma once
#include "platform/configure.h"

typedef struct SProcess SProcess;
typedef struct SProcess* SProcessHandle;
typedef int64_t SProcessId;

RUNTIME_EXTERN_C RUNTIME_API
SProcessHandle skr_run_process(const char* command, const char** arguments, uint32_t arg_count, const char* stdout_file);

RUNTIME_EXTERN_C RUNTIME_API
SProcessId skr_get_current_process_id();

RUNTIME_EXTERN_C RUNTIME_API
int skr_wait_process(SProcessHandle process);