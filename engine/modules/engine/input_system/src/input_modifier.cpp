#include "SkrInputSystem/input_modifier.hpp"

namespace skr {
namespace input {

InputValueStorage InputModifierShuffle::modify_raw(const InputValueStorage &raw) SKR_NOEXCEPT
{
    float4 v = raw.get_raw();
    float source[4] = { v.x, v.y, v.z, v.w };
    float4 result;
    result.x = source[shuffle.shuffle[0]];
    result.y = source[shuffle.shuffle[1]];
    result.z = source[shuffle.shuffle[2]];
    result.w = source[shuffle.shuffle[3]];
    return InputValueStorage(raw.get_type(), result);
}

InputValueStorage InputModifierScale::modify_raw(const InputValueStorage &raw) SKR_NOEXCEPT
{
    float4 v = raw.get_raw();
    float4 result;
    result.x = v.x * scale.x;
    result.y = v.y * scale.y;
    result.z = v.z * scale.z;
    result.w = v.w * scale.w;
    return InputValueStorage(raw.get_type(), result);
}

}}