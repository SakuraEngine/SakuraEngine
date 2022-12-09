#include "SkrToolCore/asset/cook_system.hpp"
#include "ecs/SmallVector.h"
#include "ecs/callback.hpp"
#include "ecs/dual_config.h"
#include "platform/debug.h"
#include "ecs/dual.h"
#include "resource/resource_factory.h"
#include "utils/defer.hpp"
#include "utils/log.hpp"
#include "utils/make_zeroed.hpp"
#include "ecs/type_builder.hpp"
#include "utils/types.h"
#include "SkrScene/scene.h"
#include "UsdCore/prim.hpp"
#include "SkrUsdTool/mesh_asset.hpp"
#include "SkrUsdTool/scene_asset.hpp"
#include "SkrScene/resources/scene_resource.h"

#if defined(_DEBUG) && !defined(NDEBUG)	// Use !defined(NDEBUG) to check to see if we actually are linking with Debug third party libraries (bDebugBuildsActuallyUseDebugCRT)
	#ifndef TBB_USE_DEBUG
		#define TBB_USE_DEBUG 1
	#endif
#endif

#include "UsdCore/stage.hpp"

namespace skd::asset
{

struct TranverseContext
{
    dual_storage_t* world = nullptr;
};

using children_t = llvm_vecsmall::SmallVector<dual_entity_t, 10>;
void ImportTraversal(skd::SUSDPrimId prim, TranverseContext& ctx, children_t* children)
{
    ZoneScopedN("USD ImportTraversal");
    auto usdChildren = prim->GetChildren();
    double transform[16];
    bool resetsXformStack = false;
    bool validTransform = prim->GetLocalTransformation(transform, &resetsXformStack);
    if(validTransform)
    {
        children_t ecsChildren;
        for(auto child : usdChildren)
            ImportTraversal(child, ctx, &ecsChildren);
        {
            ZoneScopedN("CreateEntity");
            dual::type_builder_t builder;
            dual_type_index_t transformType;
            if(!children)
                transformType = dual_id_of<skr_l2w_comp_t>::get();
            else
                transformType = dual_id_of<skr_l2r_comp_t>::get();
            builder.with(transformType);
            builder.with<skr_name_comp_t>();
            if(children)
                builder.with<skr_parent_comp_t>();
            if(!ecsChildren.empty())
                builder.with<skr_child_comp_t>();
            auto type = make_zeroed<dual_entity_type_t>();
            type.type = builder.build();
            auto Init = [&](dual_chunk_view_t* view)
            {
                auto self = *dualV_get_entities(view);
                auto ctransform = (skr_float4x4_t*)dualV_get_owned_ro(view, transformType);
                auto cname = dual::get_owned_rw<skr_name_comp_t>(view);
                if(!ecsChildren.empty())
                {
                    auto cchildren = dual::get_owned_rw<skr_child_comp_t, children_t>(view);
                    cchildren->resize(ecsChildren.size());
                    std::memcpy(cchildren->data(), ecsChildren.data(), ecsChildren.size() * sizeof(dual_entity_t));
                    for(auto ent : ecsChildren)
                    {
                        dual_chunk_view_t childView;
                        dualS_access(ctx.world, ent, &childView);
                        auto cparent = (skr_parent_comp_t*)dualV_get_owned_ro(&childView, dual_id_of<skr_parent_comp_t>::get());
                        cparent->entity = self;
                    }
                }
                if(children) children->push_back(self);
                if(validTransform)
                {

                    forloop(i, 0, 4)
                        forloop(j, 0, 4)
                            ctransform->M[i][j] = (float)transform[4 * i + j];
                }
                else
                    *ctransform = skr_float4x4_t();
                auto name = prim->GetName();
                size_t len = name.size();
                if(len > SKR_SCENE_MAX_NAME_LENGTH)
                {
                    SKR_LOG_WARN("Prim name is longer than the maximum allowed length. Truncating to %d characters.", SKR_SCENE_MAX_NAME_LENGTH);
                    len = SKR_SCENE_MAX_NAME_LENGTH;
                }
                memcpy(cname->str, name.c_str(), len);
                cname->str[len] = 0;
            };
            dualS_allocate_type(ctx.world, &type, 1, DUAL_LAMBDA(Init));
        }
    }
    else {
        for(auto child : usdChildren)
            ImportTraversal(child, ctx, nullptr);
    }
}

void* SSceneImporter::Import(skr_io_ram_service_t*, SCookContext* context)
{
    dual_storage_t* world = nullptr;
    auto u8Path = context->AddFileDependency(assetPath.c_str()).u8string();
    if (bool suppoted = skd::USDCoreSupportFile(u8Path.c_str()))
    {
        ZoneScopedN("USD Import");
        world = dualS_create();
        TranverseContext ctx;
        ctx.world = world;
        auto _stage = skd::USDCoreOpenStage(u8Path.c_str()); (void)_stage;
        auto _root = _stage->GetPseudoRoot();
        // pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(u8Path);
        // auto root = stage->GetPseudoRoot();
        ImportTraversal(_root, ctx, nullptr);
    }
    return world;
}

void SSceneImporter::Destroy(void* resource)
{
    auto world = (dual_storage_t*)resource;
    dualS_release(world);
}

bool SSceneCooker::Cook(SCookContext* ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    //-----load config
    // no cook config for config, skipping
    //-----import resource object
    auto world = ctx->Import<dual_storage_t>();
    SKR_DEFER({ ctx->Destroy(world); });
    //-----emit dependencies
    // TODO: static dependencies
    //-----cook resource
    // no cook needed for world now, just binarize it
    // TODO: flatten static hierarchy transform to world space to reduce runtime cost
    skr_scene_resource_t resource;
    resource.storage = world;
    //-----fetch runtime dependencies
    //TODO: iterate though all component & find resource handle fields
    eastl::vector<uint8_t> buffer;
    skr::binary::VectorWriter writer{&buffer};
    skr_binary_writer_t archive(writer);
    //------write resource object
    return ctx->Save(resource);
}

} // namespace game::asset