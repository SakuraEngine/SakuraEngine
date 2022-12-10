#pragma once
#include "SkrAssetTool/module.configure.h"
#include "containers/string.hpp"

namespace skd::asset
{
    class SImporterFactory
    {
    public:
        virtual ~SImporterFactory() = default;
        virtual bool CanImport(const skr::string& path) const = 0;
        virtual int Import(const skr::string& path) = 0;
        virtual int Update() = 0;
        virtual skr::string GetName() const = 0;
        virtual skr::string GetDescription() const = 0;
    };
}