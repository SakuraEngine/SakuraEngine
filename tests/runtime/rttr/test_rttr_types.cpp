#include "test_rttr_types.hpp"

namespace test_rttr
{
int32_t BasicRecord::static_field_int32  = {};
bool    BasicRecord::static_field_bool   = {};
float   BasicRecord::static_field_float  = {};
double  BasicRecord::static_field_double = {};
int32_t BasicRecord::_private_static_field;

float DisableMemberRTTR::static_field_a = {};
float DisableMemberRTTR::static_field_b = {};
} // namespace test_rttr