#include "types.hpp"

namespace skr {
namespace das {

#define IMPL_REG_LS(type) \
template<> type Register::Load<type>(Register r) { const vec4f& v = *(vec4f*)&r._v; return ::das::cast<type>::to(v); } \
template<> Register Register::Store<type>(type v) { const auto r = ::das::cast<type>::from(v); return *(const Register*)&r; }

IMPL_REG_LS(float);
IMPL_REG_LS(double);
IMPL_REG_LS(char);
#if defined(_MSC_VER)
IMPL_REG_LS(wchar_t);
#endif
IMPL_REG_LS(bool);
IMPL_REG_LS(vec4f);
IMPL_REG_LS(int8_t);
IMPL_REG_LS(int16_t);
IMPL_REG_LS(int32_t);
IMPL_REG_LS(int64_t);
IMPL_REG_LS(uint8_t);
IMPL_REG_LS(uint16_t);
IMPL_REG_LS(uint32_t);
IMPL_REG_LS(uint64_t);

template<> skr_float4_t Register::Load<skr_float4_t>(Register r) 
{ 
    const vec4f& v = *(vec4f*)&r._v; 
    const auto loaded = ::das::cast<vec4f>::to(v); 
    return *(skr_float4_t*)&loaded;
} 

template<> Register Register::Store<skr_float4_t>(skr_float4_t v) 
{ 
    const auto& value = *(const vec4f*)&v;
    const auto r = ::das::cast<vec4f>::from(value);
    return *(const Register*)&r; 
}

} // namespace das
} // namespace skr