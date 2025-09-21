#include <SkrBase/misc/make_zeroed.hpp>
#include <SkrBase/containers/path/path.hpp>
#include <SkrCore/memory/sp.hpp>
#include <SkrCore/module/module_manager.hpp>
#include <SkrCore/log.h>
#include <SkrCore/platform/vfs.h>
#include <SkrCore/time.h>
#include <SkrCore/async/thread_job.hpp>
#include <SkrRuntime/io/vram_io.hpp>

struct AnimSamplePlaybackModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;
};

IMPLEMENT_DYNAMIC_MODULE(AnimSamplePlaybackModule, AnimSamplePlayback);
void AnimSamplePlaybackModule::on_load(int argc, char8_t** argv)
{
    skr_log_set_level(SKR_LOG_LEVEL_INFO);
    SKR_LOG_INFO(u8"AnimSamplePlayback Module Loaded");
}

void AnimSamplePlaybackModule::on_unload()
{
    SKR_LOG_INFO(u8"AnimSamplePlayback Module Unloaded");
}

int AnimSamplePlaybackModule::main_module_exec(int argc, char8_t** argv)
{
    SKR_LOG_INFO(u8"AnimSamplePlayback Module exec");
    return 0;
}