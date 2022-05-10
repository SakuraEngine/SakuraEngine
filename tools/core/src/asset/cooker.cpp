#include "asset/cooker.hpp"
#include "utils/format.hpp"
#include "ftl/task_scheduler.h"
#include "platform/memory.h"
#include "platform/win/configure.h"
#include "json/reader.h"

namespace skd::asset
{
void SCookSystem::AddCookTask(SAssetRecord* metaAsset, ftl::TaskCounter* counter)
{
    SCookContext* jobContext = SkrNew<SCookContext>();
    jobContext->record = metaAsset;
    jobContext->system = this;
    auto Task = +[](ftl::TaskScheduler* scheduler, void* userdata) {
        SCookContext* jobContext = (SCookContext*)userdata;
        auto metaAsset = jobContext->record;
        auto outputPath = metaAsset->project->outputPath;
        ghc::filesystem::create_directories(outputPath);
        jobContext->output = outputPath / fmt::format("{}.bin", metaAsset->guid);
        auto iter = jobContext->system->cookers.find(metaAsset->type);
        SKR_ASSERT(iter != jobContext->system->cookers.end()); // TODO: error handling
        iter->second->Cook(jobContext);
    };
    scheduler.AddTask({ Task, jobContext }, ftl::TaskPriority::Normal, counter);
}
} // namespace skd::asset