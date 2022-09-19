#include "platform/debug.h"
#include "platform/process.h"
#include <EASTL/string.h>
#include <unistd.h> 

SProcessHandle skr_run_process(const char* command, const char** arguments, uint32_t arg_count, const char* stdout_file)
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}

SProcessId skr_get_current_process_id()
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return -1;
}

int skr_wait_process(SProcessHandle process)
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return -1;
}