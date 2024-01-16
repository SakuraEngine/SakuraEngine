#pragma once
#include "SkrBase/config.h"
#include "SkrBase/config.h"

typedef struct SProcess SProcess;
typedef struct SProcess* SProcessHandle;
typedef uint32_t SProcessId;

SKR_EXTERN_C SKR_CORE_API
SProcessHandle skr_run_process(const char8_t* command, const char8_t** arguments, uint32_t arg_count, const char8_t* stdout_file);

SKR_EXTERN_C SKR_CORE_API
SProcessId skr_get_current_process_id();

SKR_EXTERN_C SKR_CORE_API
const char8_t* skr_get_current_process_name();

SKR_EXTERN_C SKR_CORE_API
SProcessId skr_get_process_id(SProcessHandle);

SKR_EXTERN_C SKR_CORE_API
int skr_wait_process(SProcessHandle process);