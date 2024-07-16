#pragma once
#include "SkrSerde/json_serde.hpp"
#include "SkrToolCore/asset/cook_system.hpp"

namespace skd::asset
{
template <class T>
T LoadConfig(SCookContext* context)
{
    // TODO: now it parses twice, add cursor to reader to avoid this
    skr::archive::JsonReader reader(context->GetAssetRecord()->meta.view());
    T                        settings;
    skr::json_read(&reader, settings);
    return settings;
}
} // namespace skd::asset