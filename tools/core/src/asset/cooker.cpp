#include "asset/cooker.hpp"
#include "utils/format.hpp"
#include "ftl/task_scheduler.h"
#include "platform/memory.h"
#include "platform/configure.h"
#include "json/reader.h"

namespace skd::asset
{
auto& GetCookerRegisters()
{
    static eastl::vector<void (*)(SCookSystem*)> insts;
    return insts;
}
SCookSystem::SCookSystem()
{
    scheduler.Init();
    for (auto cookerRegister : GetCookerRegisters())
        cookerRegister(this);
}
void SCookSystem::RegisterGlobalCooker(void (*f)(SCookSystem*))
{
    GetCookerRegisters().push_back(f);
}
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

void SCookSystem::RegisterCooker(skr_guid_t guid, SCooker* cooker)
{
    SKR_ASSERT(cooker->system == nullptr);
    cooker->system = this;
    cookers.insert(std::make_pair(guid, cooker));
}
void SCookSystem::UnregisterCooker(skr_guid_t guid)
{
    cookers.erase(guid);
}
} // namespace skd::asset