#include "cgpu/cgpux.hpp"
#include "SkrRenderer/pso_map.h"
#include "SkrRenderer/render_device.h"
#include "platform/atomic.h"
#include "containers/hashmap.hpp"
#include "containers/vector.hpp"
#include "containers/sptr.hpp"
#include "utils/defer.hpp"
#include "utils/format.hpp"
#include "utils/make_zeroed.hpp"
#include "utils/threaded_service.h"

struct skr_pso_map_key_t
{
    skr_pso_map_key_t(const CGPURenderPipelineDescriptor& desc, uint64_t frame) SKR_NOEXCEPT;

    CGPURootSignatureId root_signature;
    CGPUPipelineShaderDescriptor vertex_shader;
    skr::vector<CGPUConstantSpecialization> vertex_specializations;
    CGPUPipelineShaderDescriptor tesc_shader;
    skr::vector<CGPUConstantSpecialization> tesc_specializations;
    CGPUPipelineShaderDescriptor tese_shader;
    skr::vector<CGPUConstantSpecialization> tese_specializations;
    CGPUPipelineShaderDescriptor geom_shader;
    skr::vector<CGPUConstantSpecialization> geom_specializations;
    CGPUPipelineShaderDescriptor fragment_shader;
    skr::vector<CGPUConstantSpecialization> fragment_specializations;
    CGPUVertexLayout vertex_layout;
    CGPUBlendStateDescriptor blend_state;
    CGPUDepthStateDescriptor depth_state;
    CGPURasterizerStateDescriptor rasterizer_state;
    ECGPUFormat color_formats[CGPU_MAX_MRT_COUNT];

    CGPURenderPipelineDescriptor descriptor;

    SAtomic64 frame = UINT64_MAX;
    SAtomic32 rc = 0;
    SAtomic32 pso_rc = 0;
    CGPURenderPipelineId pso = nullptr;
};

skr_pso_map_key_t::skr_pso_map_key_t(const CGPURenderPipelineDescriptor& desc, uint64_t frame) SKR_NOEXCEPT
    : root_signature(desc.root_signature->pool_sig ? desc.root_signature->pool_sig : desc.root_signature), 
    frame(frame), rc(0), pso_rc(0)
{
    if (desc.vertex_shader)
    {
        vertex_shader = *desc.vertex_shader;
        vertex_specializations = skr::vector<CGPUConstantSpecialization>(desc.vertex_shader->constants, desc.vertex_shader->constants + desc.vertex_shader->num_constants);
        descriptor.vertex_shader = &vertex_shader;
    }
    if (desc.tesc_shader)
    {
        tesc_shader = *desc.tesc_shader;
        tesc_specializations = skr::vector<CGPUConstantSpecialization>(desc.tesc_shader->constants, desc.tesc_shader->constants + desc.tesc_shader->num_constants);
        descriptor.tesc_shader = &tesc_shader;
    }
    if (desc.tese_shader)
    {
        tese_shader = *desc.tese_shader;
        tese_specializations = skr::vector<CGPUConstantSpecialization>(desc.tese_shader->constants, desc.tese_shader->constants + desc.tese_shader->num_constants);
        descriptor.tese_shader = &tese_shader;
    }
    if (desc.geom_shader)
    {
        geom_shader = *desc.geom_shader;
        geom_specializations = skr::vector<CGPUConstantSpecialization>(desc.geom_shader->constants, desc.geom_shader->constants + desc.geom_shader->num_constants);
        descriptor.geom_shader = &geom_shader;
    }
    if (desc.fragment_shader)
    {
        fragment_shader = *desc.fragment_shader;
        fragment_specializations = skr::vector<CGPUConstantSpecialization>(desc.fragment_shader->constants, desc.fragment_shader->constants + desc.fragment_shader->num_constants);
        descriptor.fragment_shader = &fragment_shader;
    }
    if (desc.vertex_layout) vertex_layout = *desc.vertex_layout;
    if (desc.blend_state) blend_state = *desc.blend_state;
    if (desc.depth_state) depth_state = *desc.depth_state;
    if (desc.rasterizer_state) rasterizer_state = *desc.rasterizer_state;

    for (uint32_t i = 0; i < CGPU_MAX_MRT_COUNT; ++i)
    {
        color_formats[i] = CGPU_FORMAT_UNDEFINED;
    }
    for (uint32_t i = 0; i < desc.render_target_count; ++i)
    {
        color_formats[i] = desc.color_formats[i];
    }
    descriptor.root_signature = root_signature;
    descriptor.vertex_layout = &vertex_layout;
    descriptor.blend_state = &blend_state;
    descriptor.depth_state = &depth_state;
    descriptor.rasterizer_state = &rasterizer_state;
}

namespace skr
{
struct PSOMapImpl : public skr_pso_map_t
{
    PSOMapImpl(const skr_pso_map_root_t& root)
        : root(root)
    {

    }

    struct key_ptr_equal
    {
        using is_transparent = void;

        size_t operator()(const SPtr<skr_pso_map_key_t>& a, const SPtr<skr_pso_map_key_t>& b) const
        {
            return cgpux::equal_to<CGPURenderPipelineDescriptor>()(a->descriptor, b->descriptor);
        }

        size_t operator()(const SPtr<skr_pso_map_key_t>& a, const CGPURenderPipelineDescriptor& b) const
        {
            return cgpux::equal_to<CGPURenderPipelineDescriptor>()(a->descriptor, b);
        }
    };

    struct key_ptr_hasher
    {
        using is_transparent = void;

        size_t operator()(const SPtr<skr_pso_map_key_t>& key) const
        {
            return cgpux::hash<CGPURenderPipelineDescriptor>()(key->descriptor);
        }

        size_t operator()(const CGPURenderPipelineDescriptor& descriptor) const
        {
            return cgpux::hash<CGPURenderPipelineDescriptor>()(descriptor);
        }
    };

    virtual skr_pso_map_key_id create_key(const struct CGPURenderPipelineDescriptor* desc) SKR_NOEXCEPT override
    {
        SKR_ASSERT(desc && "NULL descriptor not allowed!");
        auto found = sets.find(*desc);
        if (found != sets.end())
        {
            skr_pso_map_key_id result = found->get();
            skr_atomic32_add_relaxed(&result->rc, 1);
            result->frame = frame_index;
            return result;
        }
        else
        {
            auto key = SPtr<skr_pso_map_key_t>::CreateZeroed(*desc, frame_index);
            sets.insert(key);
            return key.get();
        }
    }

    virtual void free_key(skr_pso_map_key_id key) SKR_NOEXCEPT override
    {
        auto found = sets.find(key->descriptor);
        if (found != sets.end())
        {
            // deref
            skr_atomic32_add_relaxed(&key->rc, -1);
        }
        else
        {
            SKR_ASSERT(false && "Key not found!");
        }
    }
    
    struct PSORequest
    {
        PSORequest(PSOMapImpl* map, skr_pso_map_key_id key)
            : map(map), key(key)
        {

        }

        skr_async_request_t aux_request;
        PSOMapImpl* map;
        skr_pso_map_key_id key;
        SAtomic32 pso_status = 0;
    };

    ESkrPSOMapPSOStatus install_pso_impl(skr_pso_map_key_id key) SKR_NOEXCEPT
    {
        // 1. use async service install
        if (auto aux_service = root.aux_service)
        {
            auto sRequest = SPtr<PSORequest>::Create(this, key);
            auto found = mRequests.find(key);
            SKR_ASSERT(found == mRequests.end());
            mRequests.emplace(key, sRequest);

            auto aux_task = make_zeroed<skr_service_task_t>();
            aux_task.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata){
                ZoneScopedN("CreatePSO(AuxService)");
                auto sRequest = (PSORequest*)usrdata;
                auto map = sRequest->map;

                SKR_DEFER({map->mRequests.erase(sRequest->key);});

                sRequest->key->pso = cgpu_create_render_pipeline(map->root.device, &sRequest->key->descriptor);
                if (sRequest->key->pso)
                {
                    skr_atomic32_add_relaxed(&sRequest->pso_status, SKR_PSO_MAP_PSO_STATUS_INSTALLED);
                    skr_atomic64_store_relaxed(&sRequest->key->frame, map->frame_index); // store frame index to indicate pso is created
                }
                else
                {
                    skr_atomic32_add_relaxed(&sRequest->pso_status, SKR_PSO_MAP_PSO_STATUS_FAILED);
                    skr_atomic64_store_relaxed(&sRequest->key->frame, UINT64_MAX); // store frame index to indicate pso is failed
                }
            };
            aux_task.callback_datas[SKR_ASYNC_IO_STATUS_OK] = sRequest.get();
            aux_service->request(&aux_task, &sRequest->aux_request);
            return SKR_PSO_MAP_PSO_STATUS_REQUESTED;
        }
        // 2. use sync install
        else
        {
            key->pso = cgpu_create_render_pipeline(root.device, &key->descriptor);
            if (key->pso)
            {
                skr_atomic64_store_relaxed(&key->frame, frame_index); // store frame index to indicate pso is created
            }
            else
            {
                skr_atomic64_store_relaxed(&key->frame, UINT64_MAX); // store frame index to indicate pso is failed
            }
            return key->pso ? SKR_PSO_MAP_PSO_STATUS_INSTALLED : SKR_PSO_MAP_PSO_STATUS_FAILED;
        }
    }

    virtual ESkrPSOMapPSOStatus install_pso(skr_pso_map_key_id key) SKR_NOEXCEPT override
    {
        if (!key) return SKR_PSO_MAP_PSO_STATUS_FAILED;
        
        auto found = sets.find(key->descriptor);
        const auto pso_rc = skr_atomic32_load_relaxed(&key->pso_rc);
        skr_atomic32_add_relaxed(&key->pso_rc, 1);
        // 1. found mapped pso
        if (found != sets.end())
        {
            // 1.0 install pso if pso has no rc
            if (pso_rc == 0)
            {
               return install_pso_impl(key);
            }
            
            auto pFound = found->get();
            // 1.1 found alive pso request
            auto found_request = mRequests.find(key);
            if (found_request != mRequests.end())
            {
                return SKR_PSO_MAP_PSO_STATUS_REQUESTED;
            }

            // 1.2 request is done or failed
            const auto frame = skr_atomic32_load_relaxed(&pFound->frame);
            if (frame == UINT64_MAX)
            {
                return SKR_PSO_MAP_PSO_STATUS_FAILED;
            }

            // 1.3 request is done, record frame index
            skr_atomic64_store_relaxed(&pFound->frame, frame_index);
            return SKR_PSO_MAP_PSO_STATUS_INSTALLED;
        }
        // 2. not found mapped pso
        else
        {
            // keep skr_pso_map_key_t::frame at UINT64_MAX until pso is created
            return install_pso_impl(key);
        }
        return SKR_PSO_MAP_PSO_STATUS_NONE;
    }

    virtual CGPURenderPipelineId find_pso(skr_pso_map_key_id key) SKR_NOEXCEPT override
    {
        if (!key) return nullptr;

        auto found = sets.find(key->descriptor);
        if (found != sets.end())
        {
            const auto frame = skr_atomic32_load_relaxed(&found->get()->frame);
            if (frame != UINT64_MAX)
            {
                return found->get()->pso;
            }
        }
        return nullptr;
    }

    virtual bool uninstall_pso(skr_pso_map_key_id key) SKR_NOEXCEPT override
    {
        auto found = sets.find(key->descriptor);
        if (found != sets.end())
        {
        #ifdef _DEBUG
            const auto frame = skr_atomic32_load_relaxed(&found->get()->frame);
            SKR_ASSERT(frame != UINT64_MAX && "this shader is freed but never installed, check your code for errors!");
        #endif
            skr_atomic32_add_relaxed(&found->get()->pso_rc, -1);
            return true;
        }
        return false;
    }

    virtual void new_frame(uint64_t frame) SKR_NOEXCEPT override
    {
        frame_index = frame;
    }

    virtual void garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT override
    {
        for (auto it = sets.begin(); it != sets.end();)
        {
            auto key = it->get();
            if (skr_atomic32_load_relaxed(&key->rc) == 0 && skr_atomic64_load_relaxed(&key->frame) < critical_frame)
            {
                if (mRequests.find(key) != mRequests.end())
                {
                    // pso is still creating, skip & wait for it to finish
                    // TODO: cancel pso creation
                    ++it;
                }
                else
                {
                    it = sets.erase(it); // free key
                }
            }
            else
            {
                ++it;
            }
        }
        // TODO: control pso create/dealloc with pso_rc
    }

    skr_pso_map_root_t root;
    skr::parallel_flat_hash_set<SPtr<skr_pso_map_key_t>, key_ptr_hasher, key_ptr_equal> sets;
    skr::parallel_flat_hash_map<skr_pso_map_key_id, SPtr<PSORequest>> mRequests;
    SAtomic64 keys_counter = 0;
    uint64_t frame_index;
};
} // namespace skr

skr_pso_map_id skr_pso_map_t::Create(const struct skr_pso_map_root_t* root) SKR_NOEXCEPT
{
    return SkrNewZeroed<skr::PSOMapImpl>(*root);
}

bool skr_pso_map_t::Free(skr_pso_map_id pso_map) SKR_NOEXCEPT
{
    SkrDelete(pso_map);
    return true;
}

skr_pso_map_id skr_pso_map_create(const struct skr_pso_map_root_t* desc)
{
    return skr_pso_map_t::Create(desc);
}

skr_pso_map_key_id skr_pso_map_create_key(skr_pso_map_id map, const struct CGPURenderPipelineDescriptor* desc)
{
    return map->create_key(desc);
}

void skr_pso_map_free_key(skr_pso_map_id map, skr_pso_map_key_id key)
{
    map->free_key(key);
}

ESkrPSOMapPSOStatus skr_pso_map_install_pso(skr_pso_map_id map, skr_pso_map_key_id key)
{
    return map->install_pso(key);
}

CGPURenderPipelineId skr_pso_map_find_pso(skr_pso_map_id map, skr_pso_map_key_id key)
{
    return map->find_pso(key);
}

bool skr_pso_map_uninstall_pso(skr_pso_map_id map, skr_pso_map_key_id key)
{
    return map->uninstall_pso(key);
}

void skr_pso_map_new_frame(skr_pso_map_id psoMap, uint64_t frame_index)
{
    psoMap->new_frame(frame_index);
}

void skr_pso_map_garbage_collect(skr_pso_map_id psoMap, uint64_t critical_frame)
{
    psoMap->garbage_collect(critical_frame);
}

void skr_pso_map_free(skr_pso_map_id pso_map)
{
    skr_pso_map_t::Free(pso_map);
}