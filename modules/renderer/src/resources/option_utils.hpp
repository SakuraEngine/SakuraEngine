#include "utils/hash.h"
#include <EASTL/fixed_string.h>
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"

namespace option_utils
{
using opt_signature_string = eastl::fixed_string<char8_t, 64>;

inline void stringfy(opt_signature_string& string, skr::span<skr_shader_option_instance_t> ordered_options)
{
    for (auto&& option : ordered_options)
    {
        string.append(option.key.begin(), option.key.end());
        string += "=";
        string.append(option.value.begin(), option.value.end());
        string += ";";
    }
}

inline void stringfy(opt_signature_string& string, const skr_shader_option_sequence_t& seq, skr::span<uint32_t> indices)
{
    for (uint32_t i = 0; i < seq.keys.size(); i++)
    {
        const auto selection = indices[i];
        string.append(seq.keys[i].begin(), seq.keys[i].end());
        string += "=";
        string.append(seq.values[i][selection].begin(), seq.values[i][selection].end());
        string += ";";
    }
}


}