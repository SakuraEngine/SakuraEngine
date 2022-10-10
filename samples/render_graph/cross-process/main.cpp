#include "platform/process.h"
#include "utils/log.h"
#include "lmdb/lmdb.h"
#include <ghc/filesystem.hpp>
#include <EASTL/string.h>

#include "tracy/Tracy.hpp"

static const char* exec_name;

extern int provider_main(int argc, char* argv[]);
extern int receiver_main(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    exec_name = argv[0];
    if (argc == 1)
    {
        SKR_LOG_DEBUG("exec_name: %s", exec_name);
    
        const char* provider_arguments[] = { "provider", "-1"};
        auto provider = skr_run_process(exec_name, 
            provider_arguments, 2, "provider.log");
        const auto provider_id = skr_get_process_id(provider);

        eastl::string providerIdString = eastl::to_string(provider_id);
        const char* receiver_arguments[] = { "receiver", providerIdString.c_str() };
        auto receiver = skr_run_process(exec_name, 
            receiver_arguments, 2, "receiver.log");

        auto provider_result = skr_wait_process(provider);
        auto receriver_result = skr_wait_process(receiver);
        return receriver_result + provider_result;
    }
    else
    {
        auto id = skr_get_current_process_id();
        SKR_LOG_DEBUG("exec_mode: %s, process id: %lld", argv[1], id);
        auto is_receiver = (strcmp(argv[1], "receiver") == 0);

        std::error_code ec = {};
        if (!ghc::filesystem::exists("./cross-proc", ec))
        {
            SKR_LOG_INFO("subdir cross-proc not existed, create it");
            ghc::filesystem::create_directories(ghc::filesystem::path("./cross-proc"), ec);
        }

        if (is_receiver) 
            return receiver_main(argc, argv);
        else 
            return provider_main(argc, argv);
    }
}