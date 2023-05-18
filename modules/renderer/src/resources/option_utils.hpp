#include "utils/hash.h"
#include <EASTL/fixed_string.h>
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"

namespace option_utils
{
inline void stringfy(skr::string& string, skr::span<skr_shader_option_instance_t> ordered_options)
{
    for (auto&& option : ordered_options)
    {
        string += option.key;
        string += u8"=";
        string += option.value;
        string += u8";";
    }
}

inline void stringfy(skr::string& string, const skr_shader_option_sequence_t& seq, skr::span<uint32_t> indices)
{
    for (uint32_t i = 0; i < seq.keys.size(); i++)
    {
        const auto selection = indices[i];
        string += seq.keys[i];
        string += u8"=";
        string += seq.values[i][selection];
        string += u8";";
    }
}
}