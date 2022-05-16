#include "scene_asset.hpp"
#include "platform/debug.h"

namespace game::asset
{
uint32_t SSceneCooker::Version()
{
    return 0;
}

bool SSceneCooker::Cook(SCookContext* ctx)
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return true;
}

void* SSceneImporter::Import(skr::io::RAMService*, const SAssetRecord* record)
{

    return nullptr;
}
} // namespace game::asset