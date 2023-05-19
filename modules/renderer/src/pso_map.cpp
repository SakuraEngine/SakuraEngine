#include "SkrRenderer/pso_key.hpp"
#include "cgpu/cgpux.hpp"
#include "SkrRenderer/pso_map.h"
#include "SkrRenderer/render_device.h"
#include "platform/atomic.h"
#include "containers/hashmap.hpp"
#include "containers/vector.hpp"
#include "containers/sptr.hpp"
#include "misc/defer.hpp"
#include "misc/make_zeroed.hpp"
#include "async/thread_job.hpp"

#include "tracy/Tracy.hpp"

namespace skr {
namespace renderer {
struct PSOMapImpl;
namespace PSO
{

using Future = skr::IFuture<bool>;
using JobQueueFuture = skr::ThreadedJobQueueFuture<bool>;
using SerialFuture = skr::SerialFuture<bool>;
struct FutureLauncher
{
    FutureLauncher(skr::JobQueue* q) : job_queue(q) {}
    template<typename F, typename... Args>
    Future* async(F&& f, Args&&... args)
    {
        if (job_queue)
            return SkrNew<JobQueueFuture>(job_queue, std::forward<F>(f), std::forward<Args>(args)...);
        else
            return SkrNew<SerialFuture>(std::forward<F>(f), std::forward<Args>(args)...);
    }
    skr::JobQueue* job_queue = nullptr;
};

}

struct PSOProgress : public skr::AsyncProgress<PSO::FutureLauncher, int, bool>
{
    PSOProgress(PSOMapImpl* map, skr_pso_map_key_id key)
        : map(map), key(key)
    {

    }
    ~PSOProgress()
    {
        key = nullptr;
    }

    bool do_in_background() override;

    bool progress_done() const
    {
        const auto status = skr_atomicu32_load_relaxed(&key->pso_status);
        return status != SKR_PSO_MAP_PSO_STATUS_REQUESTED;
    }

    PSOMapImpl* map;
    skr_pso_map_key_id key;
};

struct PSOMapImpl : public skr_pso_map_t
{
    PSOMapImpl(const skr_pso_map_root_t& root)
        : root(root)
    {
        future_launcher = SPtr<PSO::FutureLauncher>::Create(root.job_queue);
    }

    ~PSOMapImpl()
    {
        for (auto& it : sets)
        {
            cgpu_free_render_pipeline(it->pso);
        }
    }

    struct key_ptr_equal
    {
        using is_transparent = void;

        size_t operator()(const SPtr<PSOMapKey>& a, const SPtr<PSOMapKey>& b) const
        {
            return cgpux::equal_to<CGPURenderPipelineDescriptor>()(a->descriptor, b->descriptor);
        }

        size_t operator()(const SPtr<PSOMapKey>& a, const CGPURenderPipelineDescriptor& b) const
        {
            return cgpux::equal_to<CGPURenderPipelineDescriptor>()(a->descriptor, b);
        }
    };

    struct key_ptr_hasher
    {
        using is_transparent = void;

        size_t operator()(const SPtr<PSOMapKey>& key) const
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
            skr_atomicu32_add_relaxed(&result->rc, 1);
            result->frame = frame_index;
            return result;
        }
        else
        {
            auto key = SPtr<PSOMapKey>::CreateZeroed(*desc, frame_index);
            const auto oldSize = sets.size();
            auto inserted = sets.insert(key);
            const auto newSize = sets.size();
            if (!inserted.second || oldSize == newSize)
            {
                SKR_ASSERT(0 && "Failed to insert key!");
                return nullptr;
            }
            return key.get();
        }
    }

    virtual void free_key(skr_pso_map_key_id key) SKR_NOEXCEPT override
    {
        auto found = sets.find(key->descriptor);
        if (found != sets.end())
        {
            // deref
            skr_atomicu32_add_relaxed(&key->rc, -1);
        }
        else
        {
            SKR_ASSERT(false && "Key not found!");
        }
    }

    ESkrPSOMapPSOStatus install_pso_impl(skr_pso_map_key_id key) SKR_NOEXCEPT
    {
        // 1. use async service install
        auto progress = SPtr<PSOProgress>::Create(this, key);
        mPSOProgresses.emplace(key, progress);
        skr_atomicu64_store_relaxed(&progress->key->pso_status, SKR_PSO_MAP_PSO_STATUS_REQUESTED);
        if (auto launcher = future_launcher.get()) // create shaders on aux thread
        {
            progress->execute(*launcher);
        }
        return SKR_PSO_MAP_PSO_STATUS_REQUESTED;
    }

    virtual ESkrPSOMapPSOStatus install_pso(skr_pso_map_key_id key) SKR_NOEXCEPT override
    {
        if (!key) return SKR_PSO_MAP_PSO_STATUS_FAILED;
        
        auto found = sets.find(key->descriptor);
        const auto pso_rc = skr_atomicu32_load_relaxed(&key->pso_rc);
        skr_atomicu32_add_relaxed(&key->pso_rc, 1);
        // 1. found mapped pso
        if (found != sets.end())
        {
            // query status
            auto pFound = found->get();
            const auto pso_status = skr_atomicu32_load_relaxed(&pFound->pso_status);
            // 1.0 install pso if pso has no rc
            if (pso_status == SKR_PSO_MAP_PSO_STATUS_UNINSTALLED)
            {
                if (pso_rc == 0)
                {
                    return install_pso_impl(key);
                }
                else
                {
                    SKR_UNREACHABLE_CODE();
                }
            }
            // 1.1 request is failed
            else if (pso_status == SKR_PSO_MAP_PSO_STATUS_FAILED)
            {
                return SKR_PSO_MAP_PSO_STATUS_FAILED;
            }
            // 1.2 request is done or failed
            else if (pso_status == SKR_PSO_MAP_PSO_STATUS_REQUESTED)
            {
                skr_atomicu64_store_relaxed(&pFound->frame, frame_index);
                return SKR_PSO_MAP_PSO_STATUS_REQUESTED;
            }
            // 1.3 request is done, record frame index
            else if (pso_status == SKR_PSO_MAP_PSO_STATUS_INSTALLED)
            {
                skr_atomicu64_store_relaxed(&pFound->frame, frame_index);
                return SKR_PSO_MAP_PSO_STATUS_INSTALLED;
            }
        }
        // 2. not found mapped pso
        else
        {
            // keep PSOMapKey::frame at UINT64_MAX until pso is created
            return install_pso_impl(key);
        }
        return SKR_PSO_MAP_PSO_STATUS_FAILED;
    }

    virtual CGPURenderPipelineId find_pso(skr_pso_map_key_id key) SKR_NOEXCEPT override
    {
        if (!key) return nullptr;

        const auto pso_status = skr_atomicu32_load_relaxed(&key->pso_status);
        if (pso_status == SKR_PSO_MAP_PSO_STATUS_INSTALLED)
        {
            // clearFinishedRequests();
            return key->pso;
        }
        return nullptr;
    }

    virtual bool uninstall_pso(skr_pso_map_key_id key) SKR_NOEXCEPT override
    {
        if (!key) return false;

        auto found = sets.find(key->descriptor);
        if (found != sets.end())
        {
        #ifdef _DEBUG
            const auto pso_frame = skr_atomicu32_load_relaxed(&found->get()->pso_frame);
            SKR_ASSERT(pso_frame != UINT64_MAX && "this shader is freed but never installed, check your code for errors!");
        #endif
            skr_atomicu32_add_relaxed(&found->get()->pso_rc, -1);
            return true;
        }
        return false;
    }

    virtual void new_frame(uint64_t frame) SKR_NOEXCEPT override
    {
        frame_index = frame;
    }

    void clearFinishedRequests()
    {
        for (const auto& key : sets)
        {
            mPSOProgresses.erase_if(key.get(), [](auto&& iter) {
                return iter.get()->progress_done();
            });
        }
    }

    virtual void garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT override
    {
        for (auto it = sets.begin(); it != sets.end();)
        {
            auto key = it->get();
            if (skr_atomicu32_load_relaxed(&key->rc) == 0 && skr_atomicu64_load_relaxed(&key->frame) < critical_frame)
            {
                if (mPSOProgresses.find(key) != mPSOProgresses.end())
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
        // TODO: control pso create/dealloc with pso_rc & pso_frame
        // clear finished requests
        clearFinishedRequests();
    }

    SPtr<PSO::FutureLauncher> future_launcher;
    skr_pso_map_root_t root;
    skr::parallel_flat_hash_set<SPtr<PSOMapKey>, key_ptr_hasher, key_ptr_equal> sets;
    skr::parallel_flat_hash_map<skr_pso_map_key_id, SPtr<PSOProgress>> mPSOProgresses;
    SAtomicU64 keys_counter = 0;
    uint64_t frame_index;
};

bool PSOProgress::do_in_background()
{
    ZoneScopedN("CreatePSO");

    key->pso = cgpu_create_render_pipeline(map->root.device, &key->descriptor);
    if (key->pso)
    {
        skr_atomicu64_store_relaxed(&key->pso_status, SKR_PSO_MAP_PSO_STATUS_INSTALLED);
        skr_atomicu64_store_relaxed(&key->pso_frame, map->frame_index); // store frame index to indicate pso is created
        return true;
    }
    else
    {
        skr_atomicu64_store_relaxed(&key->pso_status, SKR_PSO_MAP_PSO_STATUS_FAILED);
        skr_atomicu64_store_relaxed(&key->pso_frame, UINT64_MAX); // store frame index to indicate pso is failed
        return false;
    }
}

} // namespace renderer
} // namespace skr

skr_pso_map_id skr_pso_map_t::Create(const struct skr_pso_map_root_t* root) SKR_NOEXCEPT
{
    return SkrNewZeroed<skr::renderer::PSOMapImpl>(*root);
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