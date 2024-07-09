#pragma once
#include "SkrSerde/json/reader.h"
#include "SkrToolCore/asset/cook_system.hpp"

namespace skd::asset
{
template <class T>
T LoadConfig(SCookContext* context)
{
    skr::archive::JsonReader reader(context->GetAssetRecord()->meta.view());
    reader.StartObject();
    T settings;
    skr::json::Read(&reader, settings);
    reader.EndObject();
    return settings;
}
} // namespace skd::asset