#pragma once
#include "SkrToolCore/fwd_types.hpp"
#include "SkrToolCore/cook_system/cooker.generated.h"

namespace skd::asset
{
struct [[sattr(
    guid = "ff344604-b522-411c-b9a5-1ec4b5970c02"
)]] TOOL_CORE_API Cooker
{
    static constexpr uint32_t kDevelopmentVersion = UINT32_MAX;
    virtual ~Cooker() {}
    virtual uint32_t Version() = 0;
    virtual bool Cook(CookContext* ctx) = 0;
    CookSystem* system;
};

TOOL_CORE_API CookSystem* GetCookSystem();
TOOL_CORE_API void RegisterCookerToSystem(CookSystem* system, bool isDefault, skr::GUID cooker, skr::GUID type, Cooker* instance);

template <class T>
void RegisterCooker(bool isDefault, skr::GUID cookerGuid, skr::GUID resGuid)
{
    static T instance;
    skd::asset::RegisterCookerToSystem(GetCookSystem(), isDefault, cookerGuid, resGuid, &instance);
}
} // namespace skd::asset