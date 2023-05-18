#pragma once
#include "cook_system.hpp"
#include "serde/json/reader.h"

namespace skd::asset
{
    template<class T>
    T LoadConfig(SCookContext* context)
    {
        simdjson::ondemand::parser parser;
        auto doc = parser.iterate(context->GetAssetRecord()->meta);
        auto doc_value = doc.get_value().value_unsafe();

        T settings;
        skr::json::Read(std::move(doc_value), settings);
        return settings;
    }
}