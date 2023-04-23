#include "containers/hashmap.hpp"
#include "containers/vector.hpp"

#include "SkrRenderer/render_viewport.h"
#include "SkrRenderer/skr_renderer.h"
#include "rtm/vector4f.h"
#include "rtm/rtmx.h"

struct SViewportManagerImpl : public SViewportManager
{
    SViewportManagerImpl(dual_storage_t* storage)
    {
        camera_query = dualQ_from_literal(storage, "[in]skr_camera_comp_t");
    }

    ~SViewportManagerImpl()
    {
        dualQ_release(camera_query);
    }

    uint32_t register_viewport(const char* viewport_name) SKR_NOEXCEPT final override
    {
        auto found = idMap.find(viewport_name);
        if (found != idMap.end())
        {
            return found->second;
        }
        uint32_t idx = static_cast<uint32_t>(viewports.size());
        auto newViewport = viewports.emplace_back();
        idMap[viewport_name] = newViewport.index = idx;
        return idx;
    }

    skr_render_viewport_t* find_viewport(const char* viewport_name) SKR_NOEXCEPT final override
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

    void remove_viewport(const char* viewport_name) SKR_NOEXCEPT final override 
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
        free_list.emplace_back(index);
        viewports[index].index = UINT32_MAX;
    }

    dual_query_t* camera_query = nullptr;

    skr::parallel_flat_hash_map<skr::string, uint32_t, skr::hash<skr::string>> idMap;
    skr::vector<skr_render_viewport_t> viewports;
    skr::vector<uint32_t> free_list;
};

SViewportManager* SViewportManager::Create(dual_storage_t* storage)
{
    return SkrNew<SViewportManagerImpl>(storage);
}

void SViewportManager::Free(SViewportManager* viewport_manager)
{
    SkrDelete(viewport_manager);
}

void skr_resolve_camera_to_viewport(const skr_camera_comp_t* camera, const skr_translation_comp_t* translation, skr_render_viewport_t* viewport)
{
    SKR_ASSERT(camera->viewport_id == viewport->index && "viewport id mismatch");

    const rtm::vector4f eye = rtm::vector_load3((const uint8_t*)&translation->value);
    const rtm::vector4f camera_dir = rtm::vector_set(0.f, 1.f, 0.f, 0.f);
    const rtm::vector4f focus_pos = rtm::vector_add(eye, camera_dir);
    const auto view = rtm::look_at_matrix(
        eye /*eye*/, 
        focus_pos /*at*/,
        { 0.f, 0.f, 1.f } /*up*/
    );
    auto proj = rtm::perspective_fov(                    
        3.1415926f / 2.f, 
        (float)camera->viewport_width / (float)camera->viewport_height, 
        1.f, 1000.f);
    auto view_projection = rtm::matrix_mul(view, proj);
    
    viewport->view_projection = *(skr_float4x4_t*)&view_projection;
    viewport->viewport_width = camera->viewport_width;
    viewport->viewport_height = camera->viewport_height;
}

void skr_resolve_cameras_to_viewport(struct SViewportManager* viewport_manager, dual_storage_t* storage)
{
    dual_query_t* camera_query = static_cast<SViewportManagerImpl*>(viewport_manager)->camera_query;
    auto cameraSetup = [&](dual_chunk_view_t* g_cv) {
        ZoneScopedN("CameraResolve");

        auto cameras = dual::get_owned_rw<skr_camera_comp_t>(g_cv);
        auto camera_transforms = dual::get_owned_rw<skr_translation_comp_t>(g_cv);
        for (uint32_t i = 0; i < g_cv->count; i++)
        {
            const auto viewport_index = cameras[i].viewport_id;
            auto renderer = cameras[i].renderer;
            auto viewportManager = renderer->get_viewport_manager();
            skr_resolve_camera_to_viewport(cameras + i, camera_transforms + i, viewportManager->find_viewport(viewport_index));
        }
    };
    dualQ_get_views(camera_query, DUAL_LAMBDA(cameraSetup));
}