#pragma once
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"

namespace option_utils
{
using namespace skr;

inline void stringfy(skr::String& string, skr::Span<ShaderOptionInstance> ordered_options)
{
    for (auto&& option : ordered_options)
    {
        string.append(option.key);
        string.append(u8"=");
        string.append(option.value);
        string.append(u8";");
    }
}

inline void stringfy(skr::String& string, const ShaderOptionSequence& seq, skr::Span<uint32_t> indices)
{
    for (uint32_t i = 0; i < seq.keys.size(); i++)
    {
        const auto selection = indices[i];
        string.append(seq.keys[i]);
        string.append(u8"=");
        string.append(seq.values[i][selection]);
        string.append(u8";");
    }
}
}