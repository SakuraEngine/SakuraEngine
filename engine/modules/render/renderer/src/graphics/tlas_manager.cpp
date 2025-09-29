#include "SkrBase/atomic/atomic_mutex.hpp"
#include "SkrCore/memory/sp.hpp"
#include "SkrRenderer/render_device.h"
#include "SkrRenderer/graphics/tlas_manager.hpp"

namespace skr {

struct TLASManagerImpl;

TLASHandle::TLASHandle()
    : tlas(nullptr), pInstance(nullptr)
{
}

TLASHandle::TLASHandle(CGPUAccelerationStructureId tlas, AsyncTLASInstance* pInstance)
    : tlas(tlas), pInstance(pInstance)
{
    if (pInstance)
        pInstance->AddRef();
}

TLASHandle::TLASHandle(const TLASHandle& other)
    : tlas(other.tlas), pInstance(other.pInstance)
{
    if (pInstance)
        pInstance->AddRef();
}

TLASHandle::TLASHandle(TLASHandle&& other) noexcept
    : tlas(other.tlas), pInstance(other.pInstance)
{
    other.tlas = nullptr;
    other.pInstance = nullptr;
}

TLASHandle::~TLASHandle()
{
    if (pInstance)
    {
        pInstance->Release();
    }
}

TLASHandle& TLASHandle::operator=(const TLASHandle& other)
{
    if (this != &other)
    {
        if (pInstance)
            pInstance->Release();
        
        tlas = other.tlas;
        pInstance = other.pInstance;
        
        if (pInstance)
            pInstance->AddRef();
    }
    return *this;
}

TLASHandle& TLASHandle::operator=(TLASHandle&& other) noexcept
{
    if (this != &other)
    {
        if (pInstance)
            pInstance->Release();
        
        tlas = other.tlas;
        pInstance = other.pInstance;
        
        other.tlas = nullptr;
        other.pInstance = nullptr;
    }
    return *this;
}

struct AsyncTLASInstanceImpl : public AsyncTLASInstance
{
public:
    void Update(RenderDevice* device, CGPUCommandPoolId cmdpool, const TLASUpdateRequest& request, uint64_t version)
    {
        _state = ETLASState::Updating;
        _version = version;
        if (request.tlas_desc)
        {
            if (_tlas != nullptr)
            {
                cgpu_free_acceleration_structure(_tlas);
                _tlas = nullptr;
            }
            _tlas = cgpu_create_acceleration_structure(device->get_cgpu_device(), &request.tlas_desc.value());
        } 
        if (request.tlas_desc ||!request.blases_to_build.is_empty())       
        {
            BuildWithGPU(device, cmdpool, request);
        }
    }

    void BuildWithGPU(RenderDevice* device, CGPUCommandPoolId cmdpool, const TLASUpdateRequest& request)
    {
        auto gpu_queue = device->get_compute_queue(0) ? device->get_compute_queue(0) : device->get_gfx_queue();
        auto& blases = request.blases_to_build;
        if (_build_ctx.cmd == nullptr)
        {
            CGPUCommandBufferDescriptor cmd_desc = {
                .name = u8"CommandBuffer-BuildTLAS",
                .is_secondary = false 
            };
            _build_ctx.cmd = cgpu_create_command_buffer(cmdpool, &cmd_desc);
        }
        if (_build_ctx.build_fence == nullptr)
        {
            _build_ctx.build_fence = cgpu_create_fence(device->get_cgpu_device());
        }

        cgpu_cmd_begin(_build_ctx.cmd);
        if (!blases.is_empty())
        {
            CGPUAccelerationStructureBuildDescriptor blas_build = {
                .type = CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
                .as_count = (uint32_t)blases.size(),
                .as = blases.data()
            };
            cgpu_cmd_build_acceleration_structures(_build_ctx.cmd, &blas_build);
        }
        if (_tlas != nullptr)
        {
            // Build TLAS
            CGPUAccelerationStructureBuildDescriptor tlas_build = {
                .type = CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
                .as_count = 1,
                .as = &_tlas
            };
            cgpu_cmd_build_acceleration_structures(_build_ctx.cmd, &tlas_build);
        }
        cgpu_cmd_end(_build_ctx.cmd);

        SKR_DECLARE_ZERO(CGPUQueueSubmitDescriptor, submit);
        submit.cmds = &_build_ctx.cmd;
        submit.cmds_count = 1;
        submit.signal_fence = _build_ctx.build_fence;
        cgpu_submit_queue(gpu_queue, &submit);
    }

    TLASHandle Use(uint64_t frame)
    {
        TLASHandle handle(_tlas, this);
        last_used_frame = frame;
        return handle;
    }

    void AddRef() override
    {
        _use_count += 1;
        _state = ETLASState::InUse;
    }

    void Release() override
    {
        _use_count -= 1;
        if (_use_count == 0)
        {
            _state = ETLASState::Ready;
        }
    }

    void UpdateState()
    {
        if (_build_ctx.build_fence && _state == ETLASState::Updating)
        {
            if (cgpu_query_fence_status(_build_ctx.build_fence) == CGPU_FENCE_STATUS_COMPLETE)
            {
                _state = ETLASState::Ready;
            }
        }
    }

    AsyncTLASInstanceImpl(uint64_t v)
        : _version(v)
    {

    }

private:
    friend struct TLASManagerImpl;
    std::atomic<uint64_t> _version = 0;
    std::atomic<ETLASState> _state = ETLASState::Updating;
    CGPUAccelerationStructureId _tlas = nullptr;
    std::atomic<uint64_t> _use_count = 0;
    std::atomic<uint64_t> last_used_frame = 0;

    struct BuildContext
    {
        CGPUCommandBufferId cmd = nullptr;
        CGPUFenceId build_fence = nullptr;
    } _build_ctx;
};

struct TLASManagerImpl : public TLASManager
{
public:
    TLASManagerImpl(uint32_t max_count, RenderDevice* device)
        : device(device), max_tlas_count(max_count)
    {
        auto gpu_device = device->get_cgpu_device();
        auto gpu_queue = device->get_compute_queue(0) ? device->get_compute_queue(0) : device->get_gfx_queue();
        
        CGPUCommandPoolDescriptor pool_desc = {};
        pool_desc.name = u8"TLASManager-CommandPool";
        cmd_pool = cgpu_create_command_pool(gpu_queue, &pool_desc);
    }
    
    ~TLASManagerImpl()
    {
        // Wait for all pending operations
        for (auto&& tlas : tlases)
        {
            if (tlas->_build_ctx.build_fence)
            {
                cgpu_wait_fences(&tlas->_build_ctx.build_fence, 1);
                cgpu_free_fence(tlas->_build_ctx.build_fence);
            }
            if (tlas->_build_ctx.cmd)
            {
                cgpu_free_command_buffer(tlas->_build_ctx.cmd);
            }
            if (tlas->_tlas)
            {
                cgpu_free_acceleration_structure(tlas->_tlas);
            }
        }
        
        if (cmd_pool)
        {
            cgpu_free_command_pool(cmd_pool);
        }
    }
    
    void Request(skr::RG::RenderGraph* graph, const TLASUpdateRequest& request) override
    {
        auto tlas = GetTLASForUpdate(graph, false);
        tlas->Update(device, cmd_pool, request, version++);
    }

    TLASHandle GetLatestTLAS(skr::RG::RenderGraph* graph) const override
    {
        auto tlas = GetTLASToUse(graph);
        return tlas ? tlas->Use(graph->get_frame_index()) : TLASHandle();
    }

private:
    AsyncTLASInstanceImpl* GetTLASForUpdate(skr::RG::RenderGraph* graph, bool sync_ready) const
    {
        uint64_t oldest_version = UINT64_MAX;
        AsyncTLASInstanceImpl* oldest_tlas = nullptr;
        {
            tlases_mutex.lock_shared();
            for (auto&& tlas : tlases)
            {
                tlas->UpdateState();
                // Find the oldest (smallest version) TLAS that is Ready (not InUse)
                if (tlas->_version <= oldest_version && tlas->last_used_frame < graph->get_frame_index())
                {
                    if ((tlas->_state == ETLASState::Ready) || sync_ready)
                    {
                        oldest_version = tlas->_version;
                        oldest_tlas = tlas.get();
                    }
                }
            }
            tlases_mutex.unlock_shared();
        }
        if (sync_ready && (oldest_tlas->_state != ETLASState::Ready)) // all tlases are dirty or in use
        {
            if (oldest_tlas->_state == ETLASState::Updating) // wait last update to finish
                cgpu_wait_fences(&oldest_tlas->_build_ctx.build_fence, 1);
            if (oldest_tlas->_state == ETLASState::InUse) // wait last use to finish
                graph->wait_frame(oldest_tlas->last_used_frame);
        }
        if (!oldest_tlas)
        {
            if (tlas_count < max_tlas_count)
                oldest_tlas = CreateNewTLAS();
            else
                return GetTLASForUpdate(graph, true);
        }
        return oldest_tlas;
    }

    AsyncTLASInstanceImpl* GetTLASToUse(skr::RG::RenderGraph* graph) const
    {
        AsyncTLASInstanceImpl* chosen = nullptr;

        uint64_t oldest_tlas_version = UINT64_MAX;
        AsyncTLASInstanceImpl* oldest_tlas = nullptr;
        
        uint64_t newest_usable_version = 0;
        AsyncTLASInstanceImpl* newest_usable_tlas = nullptr;
        {
            tlases_mutex.lock_shared();
            if (tlases.is_empty())
            {
                tlases_mutex.unlock_shared();
                return nullptr;
            }

            for (auto&& tlas : tlases)
            {
                tlas->UpdateState();
                // Find the newest (largest version) TLAS that is Ready
                // Version must >= latest_used_version otherwise it will ficker
                if ((tlas->_version >= latest_used_version))
                {
                    if (tlas->_version >= newest_usable_version)
                    {
                        if ((tlas->_state == ETLASState::InUse) || (tlas->_state == ETLASState::Ready))
                        {
                            newest_usable_version = tlas->_version;
                            newest_usable_tlas = tlas.get();
                        }
                    }
                    if (tlas->_version <= oldest_tlas_version)
                    {
                        oldest_tlas_version = tlas->_version;
                        oldest_tlas = tlas.get();
                    }
                }
            }
            tlases_mutex.unlock_shared();
        }
        chosen = newest_usable_tlas;

        if (chosen == nullptr) // not found! all tlases are updating
        {
            chosen = oldest_tlas;
            if (oldest_tlas->_state == ETLASState::Updating)
                cgpu_wait_fences(&oldest_tlas->_build_ctx.build_fence, 1);
        }
        latest_used_version = chosen->_version.load();

        return chosen;
    }
    
    AsyncTLASInstanceImpl* CreateNewTLAS() const
    {
        auto tlas = skr::UPtr<AsyncTLASInstanceImpl>::New(version++);
        auto ptr = tlas.get();
        tlases_mutex.lock();
        tlases.add(std::move(tlas));
        tlas_count += 1;
        tlases_mutex.unlock();
        return ptr;
    }

    RenderDevice* device = nullptr;
    CGPUCommandPoolId cmd_pool = nullptr;

    mutable std::atomic_uint64_t latest_used_version = 0;
    mutable std::atomic_uint64_t version = 0;
    mutable shared_atomic_mutex tlases_mutex;
    mutable skr::Vector<skr::UPtr<AsyncTLASInstanceImpl>> tlases;
    
    mutable std::atomic_uint32_t tlas_count = 0;
    mutable uint32_t max_tlas_count = 3;
};

TLASManager* TLASManager::Create(uint32_t max_count, RenderDevice* device)
{
    return SkrNew<TLASManagerImpl>(max_count, device);
}

void TLASManager::Destroy(TLASManager* manager)
{
    if (manager)
    {
        SkrDelete(manager);
    }
}

} // namespace skr