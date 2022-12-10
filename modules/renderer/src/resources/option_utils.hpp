#include "utils/hash.h"
#include <EASTL/fixed_string.h>
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"

namespace option_utils
{
using opt_signature_string = eastl::fixed_string<char, 64>;

inline void stringfy(opt_signature_string& string, skr::span<skr_shader_option_instance_t> ordered_options)
{
    for (auto&& option : ordered_options)
    {
        string += option.key.c_str();
        string += "=";
        string += option.value.c_str();
        string += ";";
    }
}

inline void stringfy(opt_signature_string& string, const skr_shader_switch_sequence_t& seq, skr::span<uint32_t> indices)
{
    for (uint32_t i = 0; i < seq.keys.size(); i++)
    {
        const auto selection = indices[i];
        string += seq.keys[i].data();
        string += "=";
        string += seq.values[i][selection].data();
        string += ";";
    }
}

inline void stringfy(opt_signature_string& string, const skr_shader_option_sequence_t& seq, skr::span<uint32_t> indices)
{
    for (uint32_t i = 0; i < seq.keys.size(); i++)
    {
        const auto selection = indices[i];
        string += seq.keys[i].data();
        string += "=";
        string += seq.values[i][selection].data();
        string += ";";
    }
}


}