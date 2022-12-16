#pragma once
#include "SkrAssetTool/importer_factory.h"

#ifdef WITH_USDTOOL
namespace skd::asset
{
    SKR_ASSET_TOOL_API SImporterFactory* GetUsdImporterFactory();
}
#endif