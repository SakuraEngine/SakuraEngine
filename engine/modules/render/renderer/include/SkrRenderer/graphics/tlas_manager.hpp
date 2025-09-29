#pragma once
#include "SkrGraphics/raytracing.h"
#include "SkrContainersDef/vector.hpp"
#include "SkrContainersDef/optional.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"

namespace skr {

// Forward declarations
struct GPUScene;
struct RenderDevice;

// TLAS 生命周期状态
enum class ETLASState : uint8_t
{
    Ready,
    Updating,
    InUse 
};

struct AsyncTLASInstance
{
    virtual ~AsyncTLASInstance() = default;
    virtual void AddRef() = 0;
    virtual void Release() = 0;
};

struct SKR_RENDERER_API TLASHandle
{
public:
    TLASHandle();
    TLASHandle(CGPUAccelerationStructureId tlas, AsyncTLASInstance* pInstance);
    TLASHandle(const TLASHandle& other);
    TLASHandle(TLASHandle&& other) noexcept;
    ~TLASHandle();
    
    TLASHandle& operator=(const TLASHandle& other);
    TLASHandle& operator=(TLASHandle&& other) noexcept;

    inline operator bool() const { return tlas != nullptr; }
    CGPUAccelerationStructureId get() const { return tlas; }

private:
    CGPUAccelerationStructureId tlas = nullptr;
    AsyncTLASInstance* pInstance = nullptr;
};

struct TLASUpdateRequest
{
    skr::Optional<CGPUAccelerationStructureDescriptor> tlas_desc;
    skr::Vector<CGPUAccelerationStructureId> blases_to_build;
};

struct SKR_RENDERER_API TLASManager
{
    virtual ~TLASManager() = default;
    static TLASManager* Create(uint32_t max_count, RenderDevice* device);
    static void Destroy(TLASManager* manager);

    virtual void Request(skr::RG::RenderGraph* graph, const TLASUpdateRequest& request) = 0; 
    virtual TLASHandle GetLatestTLAS(skr::RG::RenderGraph* graph) const = 0;
};

} // namespace skr
