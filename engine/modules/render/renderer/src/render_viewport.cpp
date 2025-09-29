#include "rtm/vector4f.h"
#include "rtm/camera_utilsf.h"
#include "rtm/matrix3x4f.h"
#include "rtm/matrix4x4f.h"
#include "SkrContainers/hashmap.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrRuntime/ecs/query.hpp"
#include "SkrRenderer/render_viewport.h"
#include "SkrRenderer/skr_renderer.h"

#include "SkrProfile/profile.h"

struct SViewportManagerImpl : public SViewportManager
{
    SViewportManagerImpl(skr::ecs::ECSWorld* world)
    {
    }

    ~SViewportManagerImpl()
    {
    }

    uint32_t register_viewport(const char8_t* viewport_name) SKR_NOEXCEPT final override
    {
        auto found = idMap.find(viewport_name);
        if (found != idMap.end())
        {
            return found->second;
        }
        uint32_t idx = static_cast<uint32_t>(viewports.size());
        auto& newViewport = viewports.add_default().ref();
        idMap[viewport_name] = newViewport.index = idx;
        return idx;
    }

    skr_render_viewport_t* find_viewport(const char8_t* viewport_name) SKR_NOEXCEPT final override
    {
        auto found = idMap.find(viewport_name);
        if (found != idMap.end())
        {
            return find_viewport(found->second);
        }
        return nullptr;
    }

    skr_render_viewport_t* find_viewport(uint32_t idx) SKR_NOEXCEPT final override
    {
        const auto result = viewports.data() + idx;
        // verify if idx is removed
        SKR_ASSERT(result->index != UINT32_MAX);
        if (result->index == UINT32_MAX)
        {
            return nullptr;
        }
        return result;
    }

    void remove_viewport(const char8_t* viewport_name) SKR_NOEXCEPT final override
    {
        // TODO: verify all cameras to indicate that this viewport is safe to remove
        auto found = idMap.find(viewport_name);
        SKR_ASSERT(found != idMap.end());

        const auto index = found->second;
        idMap.erase(viewport_name);
        remove_viewport(index);
    }

    void remove_viewport(uint32_t index) SKR_NOEXCEPT final override
    {
        free_list.add(index);
        viewports[index].index = UINT32_MAX;
    }

    skr::ParallelFlatHashMap<skr::String, uint32_t, skr::Hash<skr::String>> idMap;
    skr::Vector<skr_render_viewport_t> viewports;
    skr::Vector<uint32_t> free_list;
};

SViewportManager* SViewportManager::Create(skr::ecs::ECSWorld* world)
{
    return SkrNew<SViewportManagerImpl>(world);
}

void SViewportManager::Destroy(SViewportManager* viewport_manager)
{
    SkrDelete(viewport_manager);
}

SViewportManager::~SViewportManager() SKR_NOEXCEPT
{
}