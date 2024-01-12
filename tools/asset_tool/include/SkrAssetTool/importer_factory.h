#pragma once
#include "SkrAssetTool/module.configure.h"
#include "SkrContainers/string.hpp"

namespace skd::asset
{
    class SImporterFactory
    {
    public:
        virtual ~SImporterFactory() = default;
        virtual bool CanImport(const skr::String& path) const = 0;
        virtual int Import(const skr::String& path) = 0;
        virtual int Update() = 0;
        virtual skr::String GetName() const = 0;
        virtual skr::String GetDescription() const = 0;
    };
}