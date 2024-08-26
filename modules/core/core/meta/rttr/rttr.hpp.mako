// BEGIN RTTR GENERATED
#include "SkrRTTR/rttr_traits.hpp"

// rttr traits
%for record in guid_records:
SKR_RTTR_TYPE(${record.name}, "${record.generator_data["guid"]}")
%endfor
%for enum in enums:
SKR_RTTR_TYPE(${enum.name}, "${enum.generator_data["guid"]}")
%endfor
// END RTTR GENERATED
