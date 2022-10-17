#include "platform/guid.hpp"
#include "type/type_registry.h"
#include "utils/fast_float.h"
#include <charconv>

void skr_type_t::FromString(void* dst, eastl::string_view str, skr::type::ValueSerializePolicy* policy) const
{
    using namespace skr::type;
    if (policy)
        policy->parse(policy, str, dst, this);
    else
    {
        switch (type)
        {
            case SKR_TYPE_CATEGORY_BOOL:
                if (str.compare("true") == 0)
                    *(bool*)dst = true;
                else
                    *(bool*)dst = false;
                break;
            case SKR_TYPE_CATEGORY_I32:
                std::from_chars(str.begin(), str.end(), *(int32_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_I64:
                std::from_chars(str.begin(), str.end(), *(int64_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_U32:
                std::from_chars(str.begin(), str.end(), *(uint32_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_U64:
                std::from_chars(str.begin(), str.end(), *(uint64_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_F32:
                fast_float::from_chars(str.begin(), str.end(), *(float*)dst);
                break;
            case SKR_TYPE_CATEGORY_F64:
                fast_float::from_chars(str.begin(), str.end(), *(double*)dst);
                break;
            case SKR_TYPE_CATEGORY_GUID:
                *(skr_guid_t*)dst = skr::guid::make_guid_unsafe({ str.data(), str.size() });
                break;
            case SKR_TYPE_CATEGORY_HANDLE:
                (*(skr_resource_handle_t*)dst).set_guid(skr::guid::make_guid_unsafe({ str.data(), str.size() }));
            case SKR_TYPE_CATEGORY_ENUM:
                ((const EnumType*)this)->FromString(dst, str);
                break;
            default:
                break;
        }
    }
}