#include "UsdCore/stage.hpp"

#if defined(_DEBUG) && !defined(NDEBUG)	// Use !defined(NDEBUG) to check to see if we actually are linking with Debug third party libraries (bDebugBuildsActuallyUseDebugCRT)
	#ifndef TBB_USE_DEBUG
		#define TBB_USE_DEBUG 1
	#endif
#endif
#include "pxr/base/plug/registry.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/xform.h"

#include <iostream>
#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef _WIN32
#include <direct.h>
#define curdir _getcwd
#else
#include <unistd.h>
#define curdir getcwd
#endif

void skd::OpenUSDStage(const char *path)
{
    char buff[FILENAME_MAX];
    curdir(buff, FILENAME_MAX);

    std::string pluginPath = std::string(buff) + std::string("/usd_plugins/");
    auto plugins = pxr::PlugRegistry::GetInstance().RegisterPlugins(pluginPath);
    bool m_supported = pxr::UsdStage::IsSupportedFile(path);

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