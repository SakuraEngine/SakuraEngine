#pragma once
#ifdef WITH_USDTOOL
#include "SkrAssetTool/module.configure.h"
#include "SkrUsdTool/scene_asset.hpp"

namespace skd::asset
{
    class SUsdImporterFactory
    {
    public:
        virtual bool CanImport(const skr::string& path) const = 0;
        virtual int Import(const skr::string& path) = 0;
        virtual int Update() = 0;
        virtual ~SUsdImporterFactory() = default;
    };

    SKR_ASSET_TOOL_API SUsdImporterFactory* GetUsdImporterFactory();
}
#endif