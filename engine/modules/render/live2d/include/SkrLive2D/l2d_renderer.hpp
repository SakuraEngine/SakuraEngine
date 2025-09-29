#pragma once
#include "SkrContainersDef/span.hpp"
#include "SkrRenderer/primitive_draw.h"

struct sugoi_storage_t;
namespace skr { struct RenderDevice; }
namespace skr { namespace RG { class RenderGraph; } }
namespace skr { namespace ecs { class ECSWorld; } }

namespace skr
{
 
struct SKR_LIVE2D_API Live2DRenderer
{
    static Live2DRenderer* Create();
    static void Destroy(Live2DRenderer* renderer);

    virtual ~Live2DRenderer();
    virtual void initialize(skr::RenderDevice* render_device, skr::ecs::ECSWorld* storage, struct skr_vfs_t* resource_vfs) = 0;
    virtual void finalize(skr::RenderDevice* renderer) = 0;
    virtual void produce_drawcalls(sugoi_storage_t* storage, skr::RG::RenderGraph* render_graph) = 0;
    virtual void draw(skr::RG::RenderGraph* render_graph) = 0;
};

} // namespace skr