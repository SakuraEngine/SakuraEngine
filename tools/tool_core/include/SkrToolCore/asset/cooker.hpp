#pragma once
#include "SkrToolCore/fwd_types.hpp"
#include "utils/types.h"

namespace skd sreflect
{
namespace asset sreflect
{
    struct TOOL_CORE_API SCooker {
        static constexpr uint32_t kDevelopmentVersion = UINT32_MAX;
        virtual ~SCooker() {}
        virtual uint32_t Version() = 0;
        virtual bool Cook(SCookContext* ctx) = 0;
        SCookSystem* system;
    };

    TOOL_CORE_API SCookSystem* GetCookSystem();
    TOOL_CORE_API void RegisterCookerToSystem(SCookSystem* system, skr_guid_t type, SCooker* cooker);

    #define sregister_cooker(literal) sstatic_ctor(skd::asset::RegisterCooker<$T>(skr::guid::make_guid_unsafe(literal)))

    template<class T>
    void RegisterCooker(skr_guid_t guid)
    {
        static T instance;
        skd::asset::RegisterCookerToSystem(GetCookSystem(), guid, &instance);
    }
}
}