#include "ecs/SmallVector.h"
#include "ecs/callback.hpp"
#include "ecs/dual_config.h"
#include "math/matrix.hpp"
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
            auto cname = dual::get_owned_rw<skr_name_t>(view);
            if(!ecsChildren.empty())
            {
                auto cchildren = dual::get_owned_rw<skr_child_t, children_t>(view);
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

void* SSceneImporter::Import(skr::io::RAMService*, SCookContext* context)
{
    dual_storage_t* world = nullptr;
    auto u8Path = context->AddFileDependency(assetPath.c_str()).u8string();
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

void SSceneImporter::Destroy(void* resource)
{
    auto world = (dual_storage_t*)resource;
    dualS_release(world);
}

uint32_t SSceneCooker::Version()
{
    return 0;
}

bool SSceneCooker::Cook(SCookContext* ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    //-----load config
    // no cook config for config, skipping
    //-----import resource object
    auto world = ctx->Import<dual_storage_t>();
    SKR_DEFER({ ctx->Destroy(world); });
    //-----emit dependencies
    // TODO: static dependencies
    //-----cook resource
    // no cook needed for world, just binarize it
    //-----fetch runtime dependencies
    //TODO: iterate though all component & find resource handle fields
    //-----write resource header
    eastl::vector<uint8_t> buffer;
    skr::binary::VectorWriter writer{&buffer};
    skr_binary_writer_t archive(writer);
    //------write resource object
    dualS_serialize(world, &archive);
    //------save resource to disk
    auto file = fopen(outputPath.u8string().c_str(), "wb");
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SSceneCooker::Cook] failed to write cooked file for resource {}! path: {}", 
            assetRecord->guid, assetRecord->path.u8string());
        return false;
    }
    SKR_DEFER({ fclose(file); });
    fwrite(buffer.data(), 1, buffer.size(), file);
    return true;
}

void SSceneImporterFactory::CreateImporter(const SAssetRecord *record)
{
    auto u8Path = record->path.u8string();
    //pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(u8Path);
    //SSceneImporter sceneImporter;
    //for(auto prim : stage->Traverse())
    //{
    //    if(prim.IsA<pxr::UsdGeomMesh>())
    //    {
    //        SUSDMeshImporter meshImporter;
    //    }
    //}
}

} // namespace game::asset