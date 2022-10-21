#include "scene_asset.hpp"
#include "UsdCore/prim.hpp"
#include "ecs/SmallVector.h"
#include "ecs/callback.hpp"
#include "ecs/dual_config.h"
#include "math/matrix.hpp"
#include "mesh_asset.hpp"
#include "platform/debug.h"
#include "ecs/dual.h"
#include "resource/resource_factory.h"
#include "utils/defer.hpp"
#include "utils/log.hpp"
#include "utils/make_zeroed.hpp"
#include "skr_scene/scene.h"
#include "ecs/type_builder.hpp"
#include "utils/types.h"

#if defined(_DEBUG) && !defined(NDEBUG)	// Use !defined(NDEBUG) to check to see if we actually are linking with Debug third party libraries (bDebugBuildsActuallyUseDebugCRT)
	#ifndef TBB_USE_DEBUG
		#define TBB_USE_DEBUG 1
	#endif
#endif

#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usd/primRange.h"
#include "pxr/usd/usdGeom/modelAPI.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "pxr/usd/usdGeom/mesh.h"

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
    auto usdChildren = prim->GetChildren();
    double transform[16];
    bool resetsXformStack = false;
    bool validTransform = prim->GetLocalTransformation(transform, &resetsXformStack);
    if(validTransform)
    {
        children_t ecsChildren;
        for(auto child : usdChildren)
            ImportTraversal(child, ctx, &ecsChildren);
        dual::type_builder_t builder;
        dual_type_index_t transformType;
        if(!children)
            transformType = dual_id_of<skr_l2w_t>::get();
        else
            transformType = dual_id_of<skr_l2r_t>::get();
        builder.with(transformType);
        builder.with<skr_name_t>();
        if(children)
            builder.with<skr_parent_t>();
        if(!ecsChildren.empty())
            builder.with<skr_child_t>();
        auto type = make_zeroed<dual_entity_type_t>();
        type.type = builder.build();
        auto Init = [&](dual_chunk_view_t* view)
        {
            auto self = *dualV_get_entities(view);
            auto ctransform = (skr_float4x4_t*)dualV_get_owned_ro(view, transformType);
            auto cname = (skr_name_t*)dualV_get_owned_ro(view, dual_id_of<skr_name_t>::get());
            if(!ecsChildren.empty())
            {
                auto cchildren = (skr_children_t*)dualV_get_owned_ro(view, dual_id_of<skr_child_t>::get());
                cchildren->resize(ecsChildren.size());
                std::memcpy(cchildren->data(), ecsChildren.data(), ecsChildren.size() * sizeof(dual_entity_t));
                for(auto ent : ecsChildren)
                {
                    dual_chunk_view_t childView;
                    dualS_access(ctx.world, ent, &childView);
                    auto cparent = (skr_parent_t*)dualV_get_owned_ro(&childView, dual_id_of<skr_parent_t>::get());
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
            auto len = std::min(name.size(), 31ull);
            memcpy(cname->str, name.c_str(), len);
            cname->str[len] = 0;
        };
        dualS_allocate_type(ctx.world, &type, 1, DUAL_LAMBDA(Init));
    }
    else {
        for(auto child : usdChildren)
            ImportTraversal(child, ctx, nullptr);
    }
}

void* SSceneImporter::Import(skr::io::RAMService*, const SAssetRecord* record)
{
    dual_storage_t* world = nullptr;
    auto u8Path = record->path.u8string();
    if (bool suppoted = skd::USDCoreSupportFile(u8Path.c_str()))
    {
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

uint32_t SSceneCooker::Version()
{
    return 0;
}

bool SSceneCooker::Cook(SCookContext* ctx)
{
    //-----load config
    // no cook config for config, skipping
    //-----import resource object
    auto world = ctx->Import<dual_storage_t>();
    SKR_DEFER({ dualS_release(world); });
    //-----emit dependencies
    // TODO: static dependencies
    //-----cook resource
    // no cook needed for world, just binarize it
    //-----fetch runtime dependencies
    //TODO: iterate though all component & find resource handle fields
    //-----write resource header
    eastl::vector<uint8_t> buffer;
    struct VectorWriter
    {
        eastl::vector<uint8_t>* buffer;
        int write(const void* data, size_t size)
        {
            buffer->insert(buffer->end(), (uint8_t*)data, (uint8_t*)data + size);
            return 0;
        }
    } writer{&buffer};
    skr_binary_writer_t archive(writer);
    ctx->WriteHeader(archive, this);
    //------write resource object
    dualS_serialize(world, &archive);
    //------save resource to disk
    auto file = fopen(ctx->output.u8string().c_str(), "wb");
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", ctx->record->guid, ctx->record->path.u8string());
        return false;
    }
    SKR_DEFER({ fclose(file); });
    fwrite(buffer.data(), 1, buffer.size(), file);
    return true;
}

void SSceneImporterFactory::CreateImporter(const SAssetRecord *record)
{
    auto u8Path = record->path.u8string();
    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(u8Path);
    SSceneImporter sceneImporter;
    for(auto prim : stage->Traverse())
    {
        if(prim.IsA<pxr::UsdGeomMesh>())
        {
            SUSDMeshImporter meshImporter;
        }
    }
}


} // namespace game::asset