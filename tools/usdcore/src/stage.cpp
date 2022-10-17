#include "UsdCore/stage.hpp"

#if defined(_DEBUG) && !defined(NDEBUG)	// Use !defined(NDEBUG) to check to see if we actually are linking with Debug third party libraries (bDebugBuildsActuallyUseDebugCRT)
	#ifndef TBB_USE_DEBUG
		#define TBB_USE_DEBUG 1
	#endif
#endif
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/xform.h"

#include <iostream>

void skd::OpenUSDStage(const char *path)
{
    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(path);
    auto root = stage->GetPseudoRoot();

    pxr::UsdGeomXformable xform(root);
    pxr::GfMatrix4d transform;
    bool resetsXformStack;
    bool validTransform = xform.GetLocalTransformation(&transform, &resetsXformStack);
    if(validTransform)
    {
        for (uint32_t i = 0; i < 16; i++)
        {
            std::cout << transform.data()[i] << std::endl;
        }
    }
    return;
}